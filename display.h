#ifndef DISPLAY_H_
#define DISPLAY_H_

/*
 * display.h
 *
 * Author: Coppy Nawaphanarat (44035285), Grant Wong, Will Archer
 */

#include <stdint.h>

#define MAX_ANGLE_DEGS 360

// Allows switching displaying altitude, mean, and an empty row.
// STATES_NO is always last and keeps track of the total number of display states.
enum displayStates {DISPLAY_ALTITUDE, DISPLAY_MEAN, DISPLAY_BLANK, STATES_NO};
typedef enum displayStates DisplayState;

void initDisplay(void);

void initUART(void);

void uartIntHandler(void);

void UARTSend(char *statusMessage);

void displayUART(int32_t altitude, int16_t yaw);

void displayMeanVal(int16_t meanVal, int32_t altitudePercentage, DisplayState state);

void displayYaw(int16_t yawAngle, int8_t yawDirection);

void displayRotorPWM(uint8_t mainPWMDuty, uint8_t tailPWMDuty);

#endif /* DISPLAY_H_ */
