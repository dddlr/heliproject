#ifndef FLIGHT_STATE_H_
#define FLIGHT_STATE_H_

/*
 * flightState.h
 *
 *  Author: Coppy Nawaphanarat (44035285), Grant Wong, Will Archer
 */

typedef enum {
    STARTUP_MODE,
    LANDING_MODE,
    FLYING_MODE,
    NUM_HELICOPTER_MODES
} HelicopterMode;

HelicopterMode getHelicopterMode();
void setHelicopterMode(HelicopterMode newMode);
char* getHelicopterModeString();

#endif /* FLIGHT_STATE_H_ */
