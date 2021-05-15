/*
 * slider.c - Functions relates to SW1/SW2
 *
 *
 * Author: Coppy Nawaphanarat, Grant Wong, Will Archer
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "OrbitOLED/OrbitOLEDInterface.h" // Remove once we finish with this demo on slider
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "utils/ustdlib.h"                // This too
#include <slider.h>

// *******************************************************
// Globals to module
// *******************************************************
static bool prevSliderState[NUM_SLIDERS];
static bool currSliderState[NUM_SLIDERS];

/**
 * Initialising the sliders for SW1 and SW2
 */
void initSlider(void)
{
    // Initialising SW1
    SysCtlPeripheralEnable(SW1_PERIPH);
    GPIOPinTypeGPIOInput(SLIDER_PORT_BASE, SW1_PORT_PIN);
    GPIOPadConfigSet(SLIDER_PORT_BASE, SW1_PORT_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);

    // Initialise state of the slider on TiVa board (SW1 and SW2)
    prevSliderState[SW1_SLIDER] = GPIOPinRead(SLIDER_PORT_BASE, SW1_PORT_PIN) == SW1_PORT_PIN;
    currSliderState[SW1_SLIDER] = GPIOPinRead(SLIDER_PORT_BASE, SW1_PORT_PIN) == SW1_PORT_PIN;
}

/**
 * Update the current state of the slider
 */
void updateSliders(void)
{
    int i = 0;

    // Storing the previous slider
    for (i = 0; i < NUM_SLIDERS; i++) {
        prevSliderState[i] = currSliderState[i];
    }

    currSliderState[SW1_SLIDER] = (GPIOPinRead(SLIDER_PORT_BASE, SW1_PORT_PIN) == SW1_PORT_PIN);
}

/**
 * Check the slider state
 */
sliderState checkSlider(switchSlider slider)
{
    // If the current slider state is true, set to SLIDE_UP. Otherwise, SLIDE_DOWN.
    return currSliderState[slider] ? SLIDE_UP : SLIDE_DOWN;
}
