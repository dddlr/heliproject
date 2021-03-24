//*****************************************************************************
//
// ADCdemo1.c - Simple interrupt driven program which samples with AIN0
//
// Author:  P.J. Bones	UCECE
// Last modified:	8.2.2018
//
//*****************************************************************************
// Based on the 'convert' series from 2016
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
#include "driverlib/debug.h"
#include "utils/ustdlib.h"
#include "circBufT.h"
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "buttons4.h"

// Measurements
// ============
//
// 0.01A
//
// ADC output
// 333-337 (0.27V)
// 631-634 (0.52V)
// 931-933 (0.76V)
// 1251-1253 (1.01V)
// 1869-1871 (1.50V)
// 2480-2482 (2.01V)
//
// 2541-2543 (2.92V, 2.06V showed up on power supply screen when on)
//
// OUTPUT = 1250*V (where OUTPUT is the ADC output)


//*****************************************************************************
// Constants
//*****************************************************************************

#define BUF_SIZE 8
// Assuming vertical motion of helicopter is < 4Hz
//
// 60 Hz allows the mean/buffer samples to be updated virtually instantaneously
// for the human eye
#define SAMPLE_RATE_HZ 64

//*****************************************************************************
// Global variables
//*****************************************************************************
static circBuf_t g_inBuffer;		// Buffer of size BUF_SIZE integers (sample values)
static uint32_t g_ulSampCnt;	// Counter for the interrupts

// 12-bit ADC hence the maximum value is 4095 (i.e. at 3.3V)
//
// We have a 0.8V difference so 4095 * 0.8V / 3.3V is roughly 993
static const uint32_t altitudeDelta = 993;

// Represents a 0.8V difference on the testing stations.
// Assuming PE4 pin isn't cut
//
// The ADC values we get with the testing stations are much lower than with a
// power supply, likely because of some voltage drop caused by the Tiva board
// (Number derived through experimentation with actual testing stations)
// static const uint32_t altitudeDelta = 412;

//*****************************************************************************
//
// The interrupt handler for the for SysTick interrupt.
//
//*****************************************************************************
void
SysTickIntHandler(void)
{
    // Poll the buttons
    updateButtons();
    //
    // Initiate a conversion
    //
    ADCProcessorTrigger(ADC0_BASE, 3); 
    g_ulSampCnt++;
}

//*****************************************************************************
//
// The handler for the ADC conversion complete interrupt.
// Writes to the circular buffer.
//
//*****************************************************************************
void ADCIntHandler(void)
{
	uint32_t ulValue;
	
	//
	// Get the single sample from ADC0.  ADC_BASE is defined in
	// inc/hw_memmap.h
	ADCSequenceDataGet(ADC0_BASE, 3, &ulValue);
	//
	// Place it in the circular buffer (advancing write index)
	writeCircBuf (&g_inBuffer, ulValue);
	//
	// Clean up, clearing the interrupt
	ADCIntClear(ADC0_BASE, 3);
}

//*****************************************************************************
// Initialisation functions for the clock (incl. SysTick), ADC, display
//*****************************************************************************
void initClock(void)
{
    // Set the clock rate to 20 MHz
    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);
    //
    // Set up the period for the SysTick timer.  The SysTick timer period is
    // set as a function of the system clock.
    SysTickPeriodSet(SysCtlClockGet() / SAMPLE_RATE_HZ);
    //
    // Register the interrupt handler
    SysTickIntRegister(SysTickIntHandler);
    //
    // Enable interrupt and device
    SysTickIntEnable();
    SysTickEnable();
}

void initADC(void)
{
    //
    // The ADC0 peripheral must be enabled for configuration and use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    
    // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    // will do a single sample when the processor sends a signal to start the
    // conversion.
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
  
    //
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

    //
    // Since sample sequence 3 is now configured, it must be enabled.
    ADCSequenceEnable(ADC0_BASE, 3);
  
    //
    // Register the interrupt handler
    ADCIntRegister (ADC0_BASE, 3, ADCIntHandler);
  
    //
    // Enable interrupts for ADC0 sequence 3 (clears any outstanding interrupts)
    ADCIntEnable(ADC0_BASE, 3);
}

void initDisplay(void)
{
    // Initialise the Orbit OLED display
    OLEDInitialise ();
}

//*****************************************************************************
//
// Function to display the mean ADC value (10-bit value, note) and sample count.
//
//*****************************************************************************
void displayMeanVal(uint16_t meanVal, uint32_t altitudePercentage, uint8_t state)
{
    // 16 characters across the display
    // Ensure that resulting string is empty wherever text is not added
    char string[17];
    // Form a new string for the line.  The maximum width specified for the
    //  number field ensures it is displayed right justified.

	// Display the altitude percentage
	if (state == 0) {
	    usnprintf(string, sizeof(string), "Alt. = %4d%%", altitudePercentage);
	// Display the mean ADC value
	} else if (state == 1) {
	    usnprintf(string, sizeof(string), "Mean = %5d", meanVal);
	} else {
	    // If state is 2, don't display any text.
	    usnprintf(string, sizeof(string), "                ");
	}

	// Update line on display.
    OLEDStringDraw(string, 0, 1);
}

//*****************************************************************************
// Retrieves the mean value of the buffer contents
//*****************************************************************************
uint32_t getMeanVal(void)
{
    uint16_t i;
    // Background task: calculate the (approximate) mean of the values in the
    // circular buffer and display it, together with the sample number.
    uint32_t sum = 0;
    for (i = 0; i < BUF_SIZE; i++)
        sum += readCircBuf (&g_inBuffer);

    return (2 * sum + BUF_SIZE) / 2 / BUF_SIZE;
}

int main(void)
{
	uint32_t mean = 0, landedAltitude = 0, altitudePercentage = 0;
	bool getInitHeight = true;
	uint8_t state = 0;

	initButtons();
	initClock();
	initADC();
	initDisplay();
	initCircBuf (&g_inBuffer, BUF_SIZE);

    // Enable interrupts to the processor.
    IntMasterEnable();

    // Delay by one second to give it enough time to fill up the buffer.
    // Note that SysCtlDelay runs three instructions per loop so this is
    // three times longer than necessary - this is for safety reasons.
    SysCtlDelay(SysCtlClockGet() * BUF_SIZE / SAMPLE_RATE_HZ);

	while (1) {
	    mean = getMeanVal();

	    if (checkButton(LEFT) == PUSHED) {
	        getInitHeight = true;
	    }

	    if (checkButton(UP) == PUSHED) {
	        if (++state > 2) {
	            state = 0;
	        }
	    }

	    if (getInitHeight) {
	        landedAltitude = mean;
	        getInitHeight = false;
	    }

	    altitudePercentage = (landedAltitude - mean) * 100 / altitudeDelta;

	    // Calculate and display altitude
	    displayMeanVal (mean, altitudePercentage, state);

	    // Assumes three useless instructions per "count" of the delay, hence
	    // divide by six and not two
	    //
		// SysCtlDelay (SysCtlClockGet() / 6);  // Update display at ~ 2 Hz
		SysCtlDelay (SysCtlClockGet() / 30);
	}
}

