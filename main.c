#include <stdint.h>
#include <stdbool.h>
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "buttons4.h"
#include "altitude.h"

//*****************************************************************************
// Initialisation functions for the clock (incl. SysTick), ADC, display
//*****************************************************************************
void initClock(void)
{
    // Set the clock rate to 20 MHz
    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);
    //
    // Set up the period for the SysTick timer.  The SysTick timer period is
    // set as a function of the system clock.
    SysTickPeriodSet(SysCtlClockGet() / SAMPLE_RATE_HZ);
    //
    // Register the interrupt handler
    SysTickIntRegister(SysTickIntHandler);
    //
    // Enable interrupt and device
    SysTickIntEnable();
    SysTickEnable();
}

int main(void)
{
    int32_t mean = 0, landedAltitude = 0, altitudePercentage = 0;
    bool getInitHeight = true;
    uint8_t state = 0;

    initButtons();
    initClock();
    initAltitude();
    initDisplay();

    // Enable interrupts to the processor.
    IntMasterEnable();

    // Delay by one second to give it enough time to fill up the buffer.
    // Note that SysCtlDelay runs three instructions per loop so this is
    // three times longer than necessary - this is for safety reasons.
    SysCtlDelay(SysCtlClockGet() * BUF_SIZE / SAMPLE_RATE_HZ);

    while (1) {
        mean = getMeanVal();

        if (checkButton(LEFT) == PUSHED) {
            getInitHeight = true;
        }

        if (checkButton(UP) == PUSHED) {
            if (++state > 2) {
                state = 0;
            }
        }

        if (getInitHeight) {
            landedAltitude = mean;
            getInitHeight = false;
        }

        altitudePercentage = (landedAltitude - mean) * 100 / ALTITUDE_DELTA;

        // Calculate and display altitude
        displayMeanVal (mean, altitudePercentage, state);

        // Assumes three useless instructions per "count" of the delay
        // Hence 10 Hz
        SysCtlDelay (SysCtlClockGet() / (3 * 10));
    }
}
