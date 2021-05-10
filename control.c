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

static PID altitudePID;
static PID yawPID;

static Instant prevAltitude;
static Instant currentAltitude;
static Instant prevYaw;
static Instant currentYaw;

void initPID(void)
{
    // TODO: initialise to something more sensible
    altitudePID.proportionalGain = 1;
    altitudePID.integralGain = 1;
    altitudePID.derivativeGain = 1;
}

// void pidControl(Instant* previous, Instant* current, PID constants)
void pidControl(uint32_t measuredValue, uint32_t desiredValue, PIDInput pidInput, Rotor rotor)
{
    Instant* previous;
    Instant* current;
    PID* constants;
    int32_t control = 0;

    if (pidInput == ALTITUDE) {
        previous = &prevAltitude;
        current = &currentAltitude;
        constants = &altitudePID;
    } else {
        previous = &prevYaw;
        current = &currentYaw;
        constants = &yawPID;
    }

    previous->measuredValue = current->measuredValue;
    previous->desiredValue = current->desiredValue;
    previous->errorIntegral = current->errorIntegral;

    current->measuredValue = measuredValue;
    current->desiredValue = desiredValue;
    current->error = measuredValue - desiredValue;

    float proportional = constants->proportionalGain * current->error;

    // Integral error (uses trapezoidal rule)
    current->errorIntegral = previous->errorIntegral +
            1.0/(PID_FREQUENCY) *
            (current->measuredValue + previous->measuredValue) / 2;

    float integral = constants->integralGain * current->errorIntegral;

    float differential = (current->error - previous->error) / PID_FREQUENCY;

    // TODO: figure out whether this type cast will actually work
    control = (int32_t)(proportional + integral + differential);

    if (control > 98) {
        control = 98;
    }
    if (control < 2) {
        control = 2;
    }

    setPWMDuty(control, rotor);

    // Set new value of `current` outside of this function
}
