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

static HelicopterMode helicopterMode = STARTUP_MODE;

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
    switch (helicopterMode) {
    case STARTUP_MODE:
        return "starting";
    case LANDING_MODE:
        return "landing";
    case FLYING_MODE:
        return "flying";
    }
}
