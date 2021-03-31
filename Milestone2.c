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

// clockwise
#define QUAD_CW    1
// counter-clockwise
#define QUAD_CCW   -1
// no change
#define QUAD_NULL  0
// skipped a state, indicates a problem in the code
#define QUAD_ERROR 2

int8_t yaw = 0;
int8_t aOutput = 0;
int8_t bOutput = 0;

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

void displayYaw(int8_t yaw)
{
    char string[17];
    usnprintf(string, sizeof(string), "Yaw = %4d", yaw);
    OLEDStringDraw(string, 0, 1);
}

void displayOutput(int32_t a, int32_t b)
{
    char string[17];
    usnprintf(string, sizeof(string), "a = %4d", a != 0);
    OLEDStringDraw(string, 0, 1);

    usnprintf(string, sizeof(string), "b = %4d", b != 0);
    OLEDStringDraw(string, 0, 2);
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

void yawIntHandler(void)
{
    // Clear the interrupt flags for PB0 and PB1
    GPIOIntClear(YAW_QUAD_BASE, YAW_QUAD_INT_PIN_0 | YAW_QUAD_INT_PIN_1);

    // read A and B
    aOutput = GPIOPinRead(YAW_QUAD_BASE, YAW_QUAD_INT_PIN_0);
    bOutput = GPIOPinRead(YAW_QUAD_BASE, YAW_QUAD_INT_PIN_1);
}

int main(void)
{
    initClock();
    initYaw();
    initDisplay();

    OLEDStringDraw("bloh world", 0, 2);
    OLEDStringDraw("bloh world", 0, 1);
    OLEDStringDraw("bloh world", 0, 0);

    while (1) {
        SysCtlDelay(SysCtlClockGet () / 120);
        // displayYaw(yaw);
        displayOutput(aOutput, bOutput);
    }
}
