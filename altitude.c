//*****************************************************************************
// altitude.c - Functions related to measuring the actual helicopter altitude
// (not desired altitude)
//
// Author:  Coppy Nawaphanarat, Grant Wong, Will Archer
//
//*****************************************************************************
// Description:
// Initialising the altitude-related pins, measuring samples of the
// helicopter altitude from ADC, and averaging the values to get a final
// altitude value as output.
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "circBufT.h"
#include "buttonsAPI.h"
#include "altitude.h"

//*****************************************************************************
// Global variables
//*****************************************************************************
static circBuf_t altitudeBuffer;		// Buffer of size BUF_SIZE integers (sample values)
static uint32_t sampleCount;	// Counter for the interrupts

/**
 * Initiates an ADC conversion.
 */
void ADCTrigger(void)
{
    ADCProcessorTrigger(ADC0_BASE, 3); 
    sampleCount++;
}

/**
 * The handler for the ADC conversion complete interrupt, used for getting
 * the altitude. Writes to the altitude circular buffer.
 */
void ADCIntHandler(void)
{
	uint32_t adcInput;
	

	// Get the single sample from ADC0.  ADC_BASE is defined in
	// inc/hw_memmap.h
	ADCSequenceDataGet(ADC0_BASE, 3, &adcInput);

	// Place it in the circular buffer (advancing write index)
	writeCircBuf (&altitudeBuffer, adcInput);

	// Clean up, clearing the interrupt
	ADCIntClear(ADC0_BASE, 3);
}

/**
 * Initialise the ADC peripheral and configuration
 */
void initADC(void)
{
    // The ADC0 peripheral must be enabled for configuration and use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    
    // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    // will do a single sample when the processor sends a signal to start the
    // conversion.
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
  
    // Configure step 0 on sequence 3.  Sample channel 0 (ADC_CTL_CH0) in
    // single-ended mode (default) and configure the interrupt flag
    // (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
    // that this is the last conversion on sequence 3 (ADC_CTL_END).  Sequence
    // 3 has only one programmable step.  Sequence 1 and 2 have 4 steps, and
    // sequence 0 has 8 programmable steps.  Since we are only doing a single
    // conversion using sequence 3 we will only configure step 0.  For more
    // on the ADC sequences and steps, refer to the LM3S1968 datasheet.
    //
    // Channel changed to ADC_CTL_CH9 (AIN9) to measure voltage from PE4 pin
    // (helicopter altitude sensor)
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH9 | ADC_CTL_IE |
                             ADC_CTL_END);

    // Since sample sequence 3 is now configured, it must be enabled.
    ADCSequenceEnable(ADC0_BASE, 3);

    // Register the interrupt handler
    ADCIntRegister(ADC0_BASE, 3, ADCIntHandler);
  
    // Enable interrupts for ADC0 sequence 3 (clears any outstanding interrupts)
    ADCIntEnable(ADC0_BASE, 3);
}

/**
 * Retrieves the mean value of the buffer contents
 */
uint32_t getMeanVal(void)
{
    uint16_t i;
    // Background task: calculate the (approximate) mean of the values in the
    // circular buffer and display it, together with the sample number.
    uint32_t sum = 0;
    for (i = 0; i < BUF_SIZE; i++)
        sum += readCircBuf(&altitudeBuffer);

    return (2 * sum + BUF_SIZE) / 2 / BUF_SIZE;
}

/**
 * Initialise altitude module
 */
void initAltitude(void)
{
    initADC();
    initCircBuf(&altitudeBuffer, BUF_SIZE);
}
