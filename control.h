#ifndef CONTROL_H_
#define CONTROL_H_

/*
 * control.h
 *
 * Author: Coppy Nawaphanarat (44035285), Grant Wong, Will Archer
 */

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
void pidControl(uint32_t measuredValue, uint32_t desiredValue, PIDInput pidInput, Rotor rotor);

#define PID_FREQUENCY 50.0

#endif /* CONTROL_H_ */
