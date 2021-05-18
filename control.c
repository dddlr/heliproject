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
#include "yaw.h"
#include "control.h"
#include "slider.h"

static PID altitudePID;
static PID yawPID;

static Instant prevAltitude;
static Instant currentAltitude;
static Instant prevYaw;
static Instant currentYaw;

// DEBUG
int32_t mainControl = 0;
int32_t tailControl = 0;

void initPID(void)
{
    altitudePID.proportionalGain = 1.4;
    altitudePID.integralGain = 2;
    altitudePID.derivativeGain = -50;

    yawPID.proportionalGain = 2.2;
    yawPID.integralGain = 1.2;
    yawPID.derivativeGain = -75;
    //2.2, 1.2, -75 good enough

    prevAltitude.measuredValue = 0;
    prevAltitude.desiredValue = 0;
    prevAltitude.errorIntegral = 0;
    prevAltitude.error = 0;

    currentAltitude.measuredValue = 0;
    currentAltitude.desiredValue = 0;
    currentAltitude.errorIntegral = 0;
    currentAltitude.error = 0;

    prevYaw.measuredValue = 0;
    prevYaw.desiredValue = 0;
    prevYaw.errorIntegral = 20;
    prevYaw.error = 0;

    currentYaw.measuredValue = 0;
    currentYaw.desiredValue = 0;
    currentYaw.errorIntegral = 20;
    currentYaw.error = 0;
}

int32_t angularSubtract(int32_t x, int32_t y) {
    // Output is in the range [-224,224], where 224 means x = y
    // Note that the angle is measured in number of notches (where 448 = 360 degrees)
    if (x - y < -YAW_MAX_ANGLE/2) return x - y + YAW_MAX_ANGLE;
    if (x - y > YAW_MAX_ANGLE/2)  return x - y - YAW_MAX_ANGLE;
    return x - y;
}

// void pidControl(Instant* previous, Instant* current, PID constants)
void pidControl(int32_t measuredValue, int32_t desiredValue, PIDInput pidInput, Rotor rotor)
{
    Instant* previous;
    Instant* current;
    PID* constants;
    float proportional = 0;
    float integral = 0;
    float differential = 0;
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

    if (pidInput == ALTITUDE) {
        current->error = measuredValue - desiredValue;
    } else {
        current->error = angularSubtract(desiredValue, measuredValue);
    }

    proportional = constants->proportionalGain * current->error;

    // Integral error (uses trapezoidal rule)
    current->errorIntegral = previous->errorIntegral +
            1.0/(PID_FREQUENCY) *
            (current->error + previous->error) / 2;

    integral = constants->integralGain * current->errorIntegral;

    differential = constants->derivativeGain * (current->error - previous->error) / PID_FREQUENCY;

    // TODO: figure out whether this type cast will actually work
    control = (int32_t)(proportional + integral + differential);

    if (pidInput == ALTITUDE) {
        if (control > MAX_ALTITUDE_CONTROL) control = MAX_ALTITUDE_CONTROL;
        if (control < MIN_ALTITUDE_CONTROL) control = MIN_ALTITUDE_CONTROL;

    } else {
        if (control > MAX_YAW_CONTROL) control = MAX_YAW_CONTROL;
        if (control < MIN_YAW_CONTROL) control = MIN_YAW_CONTROL;
    }

    if (rotor == ROTOR_MAIN) {
        mainControl = control;

    } else {
        tailControl = control;
    }

    setPWMDuty(control, rotor);

    // Set new value of `current` outside of this function
}

int32_t getMainControl(void)
{
    return mainControl;
}

int32_t getTailControl(void)
{
    return tailControl;
}
