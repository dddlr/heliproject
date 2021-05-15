//*****************************************************************************
// resetButton.c - Reset button
//
// Author:  Coppy Nawaphanarat, Grant Wong, Will Archer
//
//*****************************************************************************
// Description:
// Contain functions that invokes the API function SysCrlReset()
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "resetButton.h"

/**
 * Initialising the soft reset button
 */
void initResetButton(void)
{
    // Setup the pin (PA6)
    SysCtlPeripheralEnable(RESET_PERIPH);

    // Disable interrupts for safety
    GPIOIntDisable(RESET_PORT_BASE, RESET_INT_PIN);

    // Set the GPIO pin as input
    GPIOPinTypeGPIOInput(RESET_PORT_BASE, RESET_PIN);

    // Set the GPIO pin Weak Pull Up (active LOW) with 4mA
    GPIOPadConfigSet(RESET_PORT_BASE, RESET_PIN, RESET_SIG_STRENGTH, RESET_PIN_TYPE);

    // Set the GPIO pin to generate interrupts on both rising and falling edges
    GPIOIntTypeSet(RESET_PORT_BASE, RESET_PIN, RESET_EDGE_TYPE);

    // Register the interrupt handler
    GPIOIntRegister(RESET_PORT_BASE, resetButtonIntHandler);

    GPIOIntEnable(RESET_PORT_BASE, RESET_INT_PIN);
}

/*
 * The soft reset button interrupt handler
 */
void resetButtonIntHandler(void)
{
    SysCtlReset();
}
