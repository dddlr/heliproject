//*****************************************************************************
// yaw.c - Functions related to measuring the actual helicopter yaw angle
// (not the desired helicopter yaw angle)
//
// Author:  Coppy Nawaphanarat, Grant Wong, Will Archer
//
//*****************************************************************************
// Description:
// Contains functions for initialising the yaw-related pins, calculating the
// yaw angle, and quadrature decoding.
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "circBufT.h"
#include "yaw.h"

static int8_t yawDirection = 0;
static bool yawOutput[2] = {0};
static bool prevYawOutput[2] = {0};
// Yaw angle, measured in number of notches (also see YAW_MAX_ANGLE above)
static int16_t yawAngle = 0, yawRefAngle = 0;

static const int8_t quadratureLookup[16] = {
        QUAD_NULL,  QUAD_CW,    QUAD_CCW,   QUAD_ERROR,
        QUAD_CCW,   QUAD_NULL,  QUAD_ERROR, QUAD_CW,
        QUAD_CW,    QUAD_ERROR, QUAD_NULL,  QUAD_CCW,
        QUAD_ERROR, QUAD_CCW,   QUAD_CW,    QUAD_NULL
};

/**
 * Reads yaw output.
 */
void readYawOutput(void)
{
    // will either be 0 or 1
    yawOutput[0] = GPIOPinRead(YAW_QUAD_BASE, YAW_QUAD_INT_PIN_0) != 0;
    // will either be 0 or 2 (because of GPIOPinRead implementation)
    yawOutput[1] = GPIOPinRead(YAW_QUAD_BASE, YAW_QUAD_INT_PIN_1) != 0;
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

    // Get initial value of yaw output
    // (before first yaw interrupt occurs)
    //
    // This ensures that the first proper reading is correct
    readYawOutput();

    // Yaw Reference

    // Setup the pin (PC4)
    SysCtlPeripheralEnable(YAW_REF_PERIPH_GPIO);

    GPIOIntDisable(YAW_REF_BASE, YAW_REF_INT_PIN_4);

    // Set the GPIO pins as inputs
    GPIOPinTypeGPIOInput(YAW_REF_BASE, YAW_REF_PIN_4);

    // Set the GPIO pins Weak Pull Down, 2mA
    GPIOPadConfigSet(YAW_REF_BASE, YAW_REF_PIN_4,
                     YAW_QUAD_SIG_STRENGTH, YAW_QUAD_PIN_TYPE);

    // Set the GPIO pins to generate interrupts on both rising and falling edges
    GPIOIntTypeSet(YAW_REF_BASE, YAW_REF_PIN_4,
                   YAW_QUAD_EDGE_TYPE);

    // Register the interrupt handler
    GPIOIntRegister(YAW_REF_BASE, yawRefIntHandler);

    // Enable interrupts on GPIO Port B Pins 0, 1 for yaw channels A and B
    GPIOIntEnable(YAW_REF_BASE, YAW_REF_INT_PIN_4);
}

void yawRefIntHandler(void)
{
    // Something.
    GPIOIntClear(YAW_REF_BASE, YAW_REF_INT_PIN_4);

    yawRefAngle = yawAngle;
}

/**
 * Handles a change in yaw, i.e. when the yaw angle is changing.
 */
void yawIntHandler(void)
{
    // Note that the very first reading on startup won't change the yaw angle,
    // due to there being no previous yaw output to reliably determine direction

    // Clear the interrupt flags for PB0 and PB1
    GPIOIntClear(YAW_QUAD_BASE, YAW_QUAD_INT_PIN_0 | YAW_QUAD_INT_PIN_1);

    // read A and B
    prevYawOutput[0] = yawOutput[0];
    prevYawOutput[1] = yawOutput[1];

    readYawOutput();

    // Update yaw direction (clockwise, counter-clockwise etc.)
    uint8_t index = (prevYawOutput[0]<<3) + (prevYawOutput[1]<<2) +
            (yawOutput[0]<<1) + (yawOutput[1]);
    yawDirection = quadratureLookup[index];

    if ((prevYawOutput[0] != yawOutput[0] || prevYawOutput[1] != yawOutput[1]) &&
            yawDirection != QUAD_ERROR) {
        // Handles negative numbers as long as -YAW_MAX_ANGLE <= yawAngle < infinity
        //
        // Note that yawAngle is stored in number of notches, and not in degrees
        // (to avoid floating point arithmetic)
        yawAngle = (yawAngle + yawDirection + YAW_MAX_ANGLE) % YAW_MAX_ANGLE;
    }
}

int16_t getYawRefAngle(void)
{
    return yawRefAngle;
}

int16_t getYawAngle(void)
{
    return yawAngle;
}

int8_t getYawDirection(void)
{
    return yawDirection;
}
