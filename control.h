#ifndef CONTROL_H_
#define CONTROL_H_

/*
 * control.h
 *
 * Author: Coppy Nawaphanarat (44035285), Grant Wong, Will Archer
 */

#include "motor.h"
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

#define PID_FREQUENCY 120.0
// TODO: add max control for yaw as well (70%)
#define MAX_CONTROL 80
#define MIN_CONTROL 40

#endif /* CONTROL_H_ */
