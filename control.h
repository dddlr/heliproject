#ifndef CONTROL_H_
#define CONTROL_H_

/*
 * control.h
 *
 * Author: Coppy Nawaphanarat (44035285), Grant Wong, Will Archer
 */

#include "motor.h"
#include "slider.h"

typedef enum pidInput {ALTITUDE, YAW} PIDInput;

typedef struct {
    float proportionalGain;
    float integralGain;
    float derivativeGain;
} PID;

typedef struct {
    float measuredValue;
    float desiredValue;
    float errorIntegral;

    float error;
} Instant;

void initPID(void);

// void pidControl(Instant* previous, Instant* current, PID constants);
void pidControl(int32_t measuredValue, int32_t desiredValue, PIDInput pidInput, Rotor rotor);

int32_t getMainControl(void);
int32_t getTailControl(void);

#define PID_FREQUENCY 120
// Real Helicopter
#define MAX_ALTITUDE_CONTROL 60
#define MIN_ALTITUDE_CONTROL 10
#define MAX_YAW_CONTROL 40
#define MIN_YAW_CONTROL 10
#define YAW_REF_DUTY    10

// Emulator
//#define MAX_ALTITUDE_CONTROL 80
//#define MIN_ALTITUDE_CONTROL 40
//#define MAX_YAW_CONTROL 70
//#define MIN_YAW_CONTROL 20
//#define YAW_REF_DUTY    10

#endif /* CONTROL_H_ */
