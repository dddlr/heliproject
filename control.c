//*****************************************************************************
// buttons.c - Determines and stores desired altitude and yaw.
// Used by PI controller to determine how to get actual altitude and yaw to
// match desired altitude and yaw.
//
// Author:  Coppy Nawaphanarat, Grant Wong, Will Archer
//
//*****************************************************************************
// Description:
// TODO
//*****************************************************************************

#include <stdint.h>
#include <buttonsAPI.h>
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "utils/ustdlib.h"
#include "control.h"

void buttonIntHandler(void)
{
    updateButtons();
}

void initClock(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    SysTickPeriodSet(SysCtlClockGet() / 100);
    SysTickIntRegister(buttonIntHandler);

    // Enable interrupt and device
    SysTickIntEnable();
    SysTickEnable();
}

int main(void)
{
    int32_t yaw = 0, alt = 0;
    whatButton button = NUM_BUTS;

    char string[17];

    initClock();
    initButtons();
    OLEDInitialise();

    while (1) {
        button = checkWhatButton();

        switch (button) {
        case LEFT:
            yaw = (yaw - 15 + 360) % 360;
            break;

        case RIGHT:
            yaw = (yaw + 15 + 360) % 360;
            break;

        case UP:
            alt += 10;
            if (alt > 100) alt = 100;
            break;

        case DOWN:
            alt -= 10;
            if (alt < 0) alt = 0;
            break;
        }

        usnprintf(string, sizeof(string), "Alt. = %4d%%", alt);
        OLEDStringDraw(string, 0, 0);
        usnprintf(string, sizeof(string), "Yaw = %4d", yaw);
        OLEDStringDraw(string, 0, 1);
    }
}
