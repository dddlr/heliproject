#ifndef ALTITUDE_H_
#define ALTITUDE_H_

/*
 * altitude.h
 *
 * Author: Coppy Nawaphanarat (44035285), Grant Wong, Will Archer
 */

#include <stdint.h>

/**
 * Constant variables
 */

#define BUF_SIZE 8
// Assuming vertical motion of helicopter is < 4Hz
//
// 60 Hz allows the mean/buffer samples to be updated virtually instantaneously
// for the human eye
#define SAMPLE_RATE_HZ 64
// 12-bit ADC hence the maximum value is 4095 (i.e. at 3.3V)
//
// We have a 0.8V difference so 4095 * 0.8V / 3.3V is roughly 993
#define ALTITUDE_DELTA 993

// Allows switching displaying altitude, mean, and an empty row.
// STATES_NO is always last and keeps track of the total number of display states.
enum displayStates {DISPLAY_ALTITUDE, DISPLAY_MEAN, DISPLAY_BLANK, STATES_NO};
typedef enum displayStates DisplayState;

// The interrupt handler for the SysTick interrupt.
void SysTickIntHandler(void);

// The handler for the ADC conversion complete interrupt. Writes to the circular buffer.
void ADCIntHandler(void);

// Initialise functions for the clock (incl. SysTick), ADC, display
void initClock(void);

// Initialise the ADC peripheral and configuration
void initADC(void);

// Initialise the Orbit OLED display
void initDisplay(void);

// Function to display the mean ADc value (10-bit value, note) and sample count.
void displayMeanVal(int16_t meanval, int32_t altirudePercentage, uint8_t state);

// Function to retrieves the mean value of the buffer contents
uint32_t getMeanVal(void);

// Initialise altitude module
void initAltitude(void);

#endif /* ALTITUDE_H_ */
