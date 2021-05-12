//*****************************************************************************
// main.c - Helicopter project
//
// Author:  Coppy Nawaphanarat, Grant Wong, Will Archer
//
//*****************************************************************************
// Description:
// Helicopter project for ENCE361, 2021. Able to measure and display helicopter
// altitude and yaw angle.
//*****************************************************************************

#include <buttonsAPI.h>
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "altitude.h"
#include "yaw.h"
#include "display.h"
#include "motor.h"
#include "control.h"

void initClock(void)
{
    // Set the clock rate to 20 MHz
    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);

    // Set up the period for the SysTick timer.  The SysTick timer period is
    // set as a function of the system clock.
    SysTickPeriodSet(SysCtlClockGet() / SAMPLE_RATE_HZ);

    SysTickIntRegister(updateButtons);
    SysTickIntRegister(SysTickIntHandler);

    // Enable interrupt and device
    SysTickIntEnable();
    SysTickEnable();
}

int main(void)
{
    int32_t rawMeasuredAltitude = 0, rawLandedAltitude = 0;
    int32_t measuredAltitude = 0, desiredAltitude = 0, rawDesiredAltitude = 0;
    int16_t measuredYaw = 0, desiredYaw = 0;
    whatButton button = NUM_BUTS;

    bool gotInitHeight = true;
    DisplayState state = DISPLAY_ALTITUDE;

    initButtons();
    initClock();
    initAltitude();
    initYaw();
    initDisplay();
    initUART();
    initPWM();
    initPID();

    // Enable interrupts to the processor.
    IntMasterEnable();

    // Delay by one second to give it enough time to fill up the buffer.
    // Note that SysCtlDelay runs three instructions per loop so this is
    // three times longer than necessary - this is for safety reasons.
    SysCtlDelay(SysCtlClockGet() * BUF_SIZE / SAMPLE_RATE_HZ);

    // TODO replace with actual scheduler
    int8_t i = 0;

    while (1) {
        rawMeasuredAltitude = getMeanVal();
        measuredYaw = getYawAngle();

        // Milestone 1 code, TODO delete later

//        if (checkButton(LEFT) == PUSHED) {
//            gotInitHeight = true;
//        }
//
//        if (checkButton(UP) == PUSHED) {
//            if (++state >= STATES_NO) {
//                state = DISPLAY_ALTITUDE;
//            }
//        }

        if (gotInitHeight) {
            rawLandedAltitude = rawMeasuredAltitude;
            gotInitHeight = false;
        }

        //
        // Update desired altitude and yaw
        //

        button = checkWhatButton();

        switch (button) {
        case LEFT:
            desiredYaw = (desiredYaw - 15 + 360) % 360;
            break;

        case RIGHT:
            desiredYaw = (desiredYaw + 15 + 360) % 360;
            break;

        case UP:
            desiredAltitude += 10;
            if (desiredAltitude > 100) desiredAltitude = 100;
            break;

        case DOWN:
            desiredAltitude -= 10;
            if (desiredAltitude < 0) desiredAltitude = 0;
            break;
        }


        measuredAltitude = (rawLandedAltitude - rawMeasuredAltitude) * 100 / ALTITUDE_DELTA;

        displayMeanVal(rawMeasuredAltitude, measuredAltitude, state);
        displayYaw(getYawAngle(), getYawDirection());

        // Should run roughly four times every second
//        if (i % 20/4 == 0) {
            displayUART(measuredAltitude, measuredYaw, desiredAltitude, desiredYaw);
//        }

        displayRotorPWM(getPWMDuty(ROTOR_MAIN), getPWMDuty(ROTOR_TAIL));

        // TODO: Implement timer scheduler (or equivalent) and have pidControl run
        // at 50 Hz
        rawDesiredAltitude = rawLandedAltitude - (desiredAltitude) * ALTITUDE_DELTA/100;
        pidControl(rawMeasuredAltitude, rawDesiredAltitude, ALTITUDE, ROTOR_MAIN);
        pidControl(measuredYaw, desiredYaw, YAW, ROTOR_TAIL);

        // Assumes three useless instructions per "count" of the delay
        // Hence 60 Hz
        SysCtlDelay (SysCtlClockGet() / (3 * 120));

        i = (i+1) % 20;
    }
}
