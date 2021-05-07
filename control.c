//*****************************************************************************
// control.c - Determines and stores desired altitude and yaw +
// PI controller to determine how to get actual altitude and yaw to
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
#include "motor.h"
#include "control.h"

typedef struct {
    float proportionalGain;
    float integralGain;
    float derivativeGain = 0;
} PID;

typedef struct {
    float measuredValue;
    float desiredValue;
    float errorIntegral = 0;

    float error = 0;
} Instant;

static PID altitudePID;
static PID yawPID;

static Instant prevAltitude;
static Instant currentAltitude;
static Instant prevYaw;
static Instant currentYaw;


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

void initPID(void)
{
    // TODO: initialise global variables
}

void pidControl(Instant* previous, Instant* current, PID constants)
{
    int32_t control = 0;
    current->error = current->desiredValue - current->measuredValue;
    float proportional = constants.proportionalGain * current->error;
    // Integral error
    //
    // Uses trapezoidal rule
    current->errorIntegral += PID_FREQUENCY *
            (current->measuredValue + previous->measuredValue) / 2;

    float integral = constants.integralGain * current->errorIntegral;

    float differential = (current->error - previous->error) / PID_FREQUENCY;

    // TODO: figure out whether this type cast will actually work
    control = (int32_t)(proportional + integral + differential);

    if (control > 98) {
        control = 98;
    }
    if (control < 2) {
        control = 2;
    }

    setPWMDuty(control, ROTOR_MAIN);

    previous->measuredValue = current->measuredValue;
    previous->desiredValue = current->desiredValue;
    previous->errorIntegral = current->errorIntegral;

    // Set new value of `current` outside of this function
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
