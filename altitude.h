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

#define ADC_TRIGGER_FREQ 200

// 12-bit ADC hence the maximum value is 4095 (i.e. at 3.3V)
//
// We have a 0.8V difference so 4095 * 0.8V / 3.3V is roughly 993
#define ALTITUDE_DELTA 993

// Initiates an ADC conversion.
void ADCTrigger(void);

// The handler for the ADC conversion complete interrupt. Writes to the circular buffer.
void ADCIntHandler(void);

// Initialise functions for the clock (incl. SysTick), ADC, display
void initClock(void);

// Initialise the ADC peripheral and configuration
void initADC(void);

// Function to display the mean ADc value (10-bit value, note) and sample count.
void displayMeanVal(int16_t meanval, int32_t altirudePercentage, uint8_t state);

// Function to retrieves the mean value of the buffer contents
uint32_t getMeanVal(void);

// Initialise altitude module
void initAltitude(void);

#endif /* ALTITUDE_H_ */
