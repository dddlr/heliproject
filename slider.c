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

// Slider MAIN base
#define SLIDER_PORT_BASE  GPIO_PORTA_BASE

// SW1: PA7 (J1-10)
#define SW1_PERIPH        SYSCTL_PERIPH_GPIOA   //At A-Pin
#define SW1_PORT_PIN      GPIO_PIN_7

// SW2: PA6 (J1-09)
#define SW2_PERIPH        SYSCTL_PERIPH_GPIOA
#define SW2_PORT_PIN      GPIO_PIN_6

// *******************************************************
// Globals to module
// *******************************************************
static bool prevSliderState[NUM_SLIDERS];
static bool currSliderState[NUM_SLIDERS];

void initClock(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    SysTickPeriodSet(SysCtlClockGet() / 100);
    SysTickIntRegister(sliderIntHandler);

    // Enable interrupt and device
    SysTickIntEnable();
    SysTickEnable();
}

/**
 * Initialising the sliders for SW1 and SW2
 */
void initSlider(void)
{
    // Initialising SW1
    SysCtlPeripheralEnable(SW1_PERIPH);
    GPIOPinTypeGPIOInput(SLIDER_PORT_BASE, SW1_PORT_PIN);
    GPIOPadConfigSet(SLIDER_PORT_BASE, SW1_PORT_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);

    // Initialising SW2
    SysCtlPeripheralEnable(SW2_PERIPH);
    GPIOPinTypeGPIOInput(SLIDER_PORT_BASE, SW2_PORT_PIN);
    GPIOPadConfigSet(SLIDER_PORT_BASE, SW2_PORT_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);

    // Initialise state of the slider on TiVa board (SW1 and SW2)
    prevSliderState[SW1_SLIDER] = GPIOPinRead(SLIDER_PORT_BASE, SW1_PORT_PIN) == SW1_PORT_PIN;
    currSliderState[SW1_SLIDER] = GPIOPinRead(SLIDER_PORT_BASE, SW1_PORT_PIN) == SW1_PORT_PIN;
    prevSliderState[SW2_SLIDER] = GPIOPinRead(SLIDER_PORT_BASE, SW2_PORT_PIN) == SW2_PORT_PIN;
    currSliderState[SW2_SLIDER] = GPIOPinRead(SLIDER_PORT_BASE, SW2_PORT_PIN) == SW2_PORT_PIN;
}

/**
 * Interrupt handler for the slider
 */
void sliderIntHandler(void)
{
    updateSlider();
}

/**
 * Update the current state of the slider
 */
void updateSlider(void)
{
    int i = 0;

    // Storing the previous slider
    for (i = 0; i < NUM_SLIDERS; i++) {
        prevSliderState[i] = currSliderState[i];
    }

    currSliderState[SW1_SLIDER] = (GPIOPinRead(SLIDER_PORT_BASE, SW1_PORT_PIN) == SW1_PORT_PIN);
    currSliderState[SW2_SLIDER] = (GPIOPinRead(SLIDER_PORT_BASE, SW2_PORT_PIN) == SW2_PORT_PIN);
}

/**
 * Check the slider state
 */
sliderState checkSlider(switchSlider slider)
{
    // If the current slider state is true, set to SLIDE_UP. Otherwise, SLIDE_DOWN.
    // TODO: Do we need to make it more clearer version of this?
    return currSliderState[slider] ? SLIDE_UP : SLIDE_DOWN;
}

int main(void)
{
    initClock();
    initSlider();
    OLEDInitialise();
    sliderState slider = NOCHANGE;
    char string[17];

    while (1) {
        slider = checkSlider(SW1_SLIDER);
        switch (slider) {
        case SLIDE_UP:
            usnprintf(string, sizeof(string), "SW1 = FLYING ");
            break;
        case SLIDE_DOWN:
            usnprintf(string, sizeof(string), "SW1 = LANDING");
        }
        OLEDStringDraw(string, 0, 0);
    }
}
