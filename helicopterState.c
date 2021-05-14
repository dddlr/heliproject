//*****************************************************************************
// flightState.c - Stores whether the helicopter is flying, landing etc.
//
// Author:  Coppy Nawaphanarat, Grant Wong, Will Archer
//
//*****************************************************************************
// Description:
// TODO
//*****************************************************************************

#include "helicopterState.h"

static HelicopterMode helicopterMode = LANDING_MODE;

HelicopterMode getHelicopterMode()
{
    return helicopterMode;
}

void setHelicopterMode(HelicopterMode newMode)
{
    helicopterMode = newMode;
}

char* getHelicopterModeString()
{
    if (helicopterMode == LANDING_MODE) {
        return "landing";
    }
    return "flying";
}
