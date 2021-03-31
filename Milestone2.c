/*
 * Milestone2.c - The second milestone task for the helicopter project
 *
 *  Created on: 31/03/2021
 *  Authors: Suwijuk Nawaphanarat (44035285), Grant Wong, Will Archer
 *
 *  Description:
 *  This module contains the second milestone task for the helicopter project.
 *  It is for calculating the yaw values, the slot counts, and initialiser of the
 *  quadrature state machine.
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "utils/ustdlib.h"
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "circBufT.h"

//*****************************************************************************
// Constants
//
// Yaw Quadrature Encoding:
// PB0 is the Channel/Phase A pin
// PB1 is the Channel/Phase B pin
//*****************************************************************************

/*
 * Yaw configuration
 */
#define YAW_QUAD_PERIPH_GPIO    SYSCTL_PERIPH_GPIOB
#define YAW_QUAD_BASE           GPIO_PORTB_BASE
#define YAW_QUAD_INT_PIN_0      GPIO_INT_PIN_0
#define YAW_QUAD_PIN_0          GPIO_PIN_0
#define YAW_QUAD_INT_PIN_1      GPIO_INT_PIN_1
#define YAW_QUAD_PIN_1          GPIO_PIN_1
#define YAW_QUAD_SIG_STRENGTH   GPIO_STRENGTH_4MA
#define YAW_QUAD_PIN_TYPE       GPIO_PIN_TYPE_STD_WPD
#define YAW_QUAD_EDGE_TYPE      GPIO_BOTH_EDGES
#define YAW_QUAD_DDR            GPIO_DIR_MODE_IN

#define YAW_TOTAL_NOTCHES       110
// Yaw angle is measured in number of notches, using
// quadrature decoding (hence multiply by 4)
#define YAW_MAX_ANGLE           (YAW_TOTAL_NOTCHES * 4)

// clockwise
#define QUAD_CW    1
// counter-clockwise
#define QUAD_CCW   -1
// no change
#define QUAD_NULL  0
// skipped a state, indicates a problem in the code
#define QUAD_ERROR 2

int8_t yawDirection = 0;
// Yaw angle, measured in number of notches (also see YAW_MAX_ANGLE above)
int16_t yawAngle = 0;
bool yawOutput[2] = {0};
bool prevYawOutput[2] = {0};

int8_t quadratureLookup[16] = {
        QUAD_NULL,  QUAD_CW,    QUAD_CCW,   QUAD_ERROR,
        QUAD_CCW,   QUAD_NULL,  QUAD_ERROR, QUAD_CW,
        QUAD_CW,    QUAD_ERROR, QUAD_NULL,  QUAD_CCW,
        QUAD_ERROR, QUAD_CCW,   QUAD_CW,    QUAD_NULL
};

// Prototype functions
void initYaw(void);
void yawIntHandler(void);

void initClock(void)
{
    // Set the clock rate to 20 MHz
    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);
}

void initDisplay(void)
{
    // Initialise the Orbit OLED display
    OLEDInitialise();
}

void initYaw(void)
{
    // setup the pins (PB0 is A, PB1 is B)
    SysCtlPeripheralEnable(YAW_QUAD_PERIPH_GPIO);

    // disable interrupts for safety
    // TODO: test whether this is necessary
    GPIOIntDisable(YAW_QUAD_BASE, YAW_QUAD_INT_PIN_0 | YAW_QUAD_INT_PIN_1);

    // Set the GPIO pins as inputs
    GPIOPinTypeGPIOInput(YAW_QUAD_BASE, YAW_QUAD_PIN_0 | YAW_QUAD_PIN_1);

    // Set the GPIO pins Weak Pull Down, 2mA
    GPIOPadConfigSet(YAW_QUAD_BASE, YAW_QUAD_PIN_0 | YAW_QUAD_PIN_1,
                     YAW_QUAD_SIG_STRENGTH, YAW_QUAD_PIN_TYPE);

    // Set the GPIO pins to generate interrupts on both rising and falling edges
    GPIOIntTypeSet(YAW_QUAD_BASE, YAW_QUAD_PIN_0 | YAW_QUAD_PIN_1,
                   YAW_QUAD_EDGE_TYPE);

    // Register the interrupt handler
    GPIOIntRegister(YAW_QUAD_BASE, yawIntHandler);

    // Enable interrupts on GPIO Port B Pins 0, 1 for yaw channels A and B
    GPIOIntEnable(YAW_QUAD_BASE, YAW_QUAD_INT_PIN_0 | YAW_QUAD_INT_PIN_1);
}

/**
 * Handles a change in yaw, i.e. when the yaw angle is changing.
 */
void yawIntHandler(void)
{
    // Clear the interrupt flags for PB0 and PB1
    GPIOIntClear(YAW_QUAD_BASE, YAW_QUAD_INT_PIN_0 | YAW_QUAD_INT_PIN_1);

    // read A and B
    prevYawOutput[0] = yawOutput[0];
    prevYawOutput[1] = yawOutput[1];
    // will either be 0 or 1
    yawOutput[0] = GPIOPinRead(YAW_QUAD_BASE, YAW_QUAD_INT_PIN_0) != 0;
    // will either be 0 or 2 (because of GPIOPinRead implementation)
    yawOutput[1] = GPIOPinRead(YAW_QUAD_BASE, YAW_QUAD_INT_PIN_1) != 0;

    // Update yaw direction (clockwise, counter-clockwise etc.)
    uint8_t index = (prevYawOutput[0]<<3) + (prevYawOutput[1]<<2) +
            (yawOutput[0]<<1) + (yawOutput[1]);
    yawDirection = quadratureLookup[index];

    if (prevYawOutput[0] != yawOutput[0] || prevYawOutput[1] != yawOutput[1] &&
            yawDirection != QUAD_ERROR) {
        // Handles negative numbers as long as -YAW_MAX_ANGLE <= yawAngle < infinity
        //
        // Note that yawAngle is stored in number of notches, and not in degrees
        // (to avoid floating point arithmetic)
        yawAngle = (yawAngle + yawDirection + YAW_MAX_ANGLE) % YAW_MAX_ANGLE;
    }
}

void displayYaw(int8_t yawDirection)
{
    char string[17];
    // Note integer division below loses a bit of accuracy and always rounds down
    usnprintf(string, sizeof(string), "YawAngle = %5d", 360*yawAngle/YAW_MAX_ANGLE);
    OLEDStringDraw(string, 0, 3);

    if (yawDirection == QUAD_CW) {
        OLEDStringDraw("Yaw ClockWise   ", 0, 0);
    } else if (yawDirection == QUAD_CCW) {
        OLEDStringDraw("Yaw CntClockwise", 0, 0);
    } else if (yawDirection == QUAD_NULL) {
        OLEDStringDraw("Yaw No Change   ", 0, 0);
    } else {
        // indicates a yaw measurement interrupt has been skipped - this
        // should never happen
        OLEDStringDraw("Yaw ERROR ERROR ", 0, 0);
    }
}

void displayOutput(int32_t a, int32_t b)
{
    char string[17];
    usnprintf(string, sizeof(string), "a = %4d", a);
    OLEDStringDraw(string, 0, 1);

    usnprintf(string, sizeof(string), "b = %4d", b);
    OLEDStringDraw(string, 0, 2);
}

int main(void)
{
    initClock();
    initYaw();
    initDisplay();

    while (1) {
        SysCtlDelay(SysCtlClockGet () / 120);
        displayYaw(yawDirection);
        displayOutput(yawOutput[0], yawOutput[1]);
    }
}
