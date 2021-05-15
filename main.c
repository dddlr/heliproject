//*****************************************************************************
// main.c - Helicopter project
//
// Author:  Coppy Nawaphanarat, Grant Wong, Will Archer
//
//*****************************************************************************
// Description:
// Helicopter project for ENCE361, 2021. Able to measure and display helicopter
// altitude and yaw angle.
//*****************************************************************************

#include <buttonsAPI.h>
#include <helicopterState.h>
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "altitude.h"
#include "yaw.h"
#include "display.h"
#include "motor.h"
#include "control.h"
#include "slider.h"

#define MAX_SAMPLE_RATE 500
#define ALTITUDE_NOT_FOUND -1

typedef enum {
    TASK_ADC,
    TASK_BUTTON_POLLING,
    TASK_DISPLAY,
    TASK_UART,
    TASK_BUTTON_STATUS,
    TASK_PID,
    N_TASKS
} Task;

typedef struct {
    int16_t period;
    int16_t timeTilRun;
    // Time to wait before starting the execution of this task,
    // where MAX_SAMPLE_RATE is one second.
    int16_t handicap;
    bool ready;
} TaskStatus;

// Each value must be less than or equal to MAX_SAMPLE_RATE
static uint16_t taskFreqs[N_TASKS] = {ADC_TRIGGER_FREQ, 100, 50, 5, 30, PID_FREQUENCY};
// Delay PID calculations by 1 second (MAX_SAMPLE_RATE) to give
// ADC time to populate its buffer and return correct averaged altitudes
//
// TODO: replace this with a general delay on ALL the tasks
// to wait for ADC buffer to fill up AND for the heli to figure out
// where the reference angle is
// static uint16_t taskHandicaps[N_TASKS] = {0, 0, 0, 0, 0, MAX_SAMPLE_RATE*10};
static uint16_t taskHandicaps[N_TASKS] = {0}; // disable handicaps for the time being
static TaskStatus scheduledTasks[N_TASKS] = {};

void timerInterruptHandler(void)
{
    uint16_t i = 0;
    for (; i < N_TASKS; i++) {
        if (scheduledTasks[i].handicap > 0) {
            --scheduledTasks[i].handicap;
            continue;
        }

        if (scheduledTasks[i].timeTilRun == 0) {
            scheduledTasks[i].ready = true;
            // Reset task
            scheduledTasks[i].timeTilRun = scheduledTasks[i].period;
        } else {
            --scheduledTasks[i].timeTilRun;
        }
    }
}

void initClock(void)
{
    // Set the clock rate to 20 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);

    // Set up the period for the SysTick timer. The SysTick timer period is
    // set as a function of the system clock.
    SysTickPeriodSet(SysCtlClockGet() / MAX_SAMPLE_RATE);
    SysTickIntRegister(timerInterruptHandler);

    // Enable interrupt and device
    SysTickIntEnable();
    SysTickEnable();
}

void initTasks(void)
{
    uint16_t i = 0;
    for (; i < N_TASKS; ++i) {
        scheduledTasks[i].period = MAX_SAMPLE_RATE / taskFreqs[i];
        scheduledTasks[i].timeTilRun = MAX_SAMPLE_RATE / taskFreqs[i];
        scheduledTasks[i].handicap = taskHandicaps[i];
        scheduledTasks[i].ready = false;
    }
}

int main(void)
{
    int32_t rawMeasuredAltitude = ALTITUDE_NOT_FOUND;
    int32_t rawLandedAltitude = ALTITUDE_NOT_FOUND;
    int32_t measuredAltitude = ALTITUDE_NOT_FOUND;
    int32_t desiredAltitude = 0;
    int32_t rawDesiredAltitude = 0;
    int32_t measuredYaw = -1, desiredYaw = 0, rawDesiredYaw = 0;

    whatButton button = NUM_BUTS;
    sliderState slider = NUM_SLIDER_STATES;

    // Whether it is necessary to run the functions related to
    // initialising reference altitude and yaw
    // TODO: possibly move to helicopterState.c
    bool needFetchInitValues = true;
    // Whether the helicopter is landed
    // TODO: move to helicopterState.c
    bool landed = true;

    DisplayState state = DISPLAY_ALTITUDE;

    initButtons();
    initSlider();
    initClock();
    initAltitude();
    initYaw();
    initDisplay();
    initUART();
    initPWM();
    initPID();
    initTasks();
    initResetButton();

    // Enable interrupts to the processor.
    IntMasterEnable();

    while (1) {
        if (scheduledTasks[TASK_ADC].ready) {
            ADCTrigger();

            scheduledTasks[TASK_ADC].ready = false;
        }

        if (scheduledTasks[TASK_BUTTON_POLLING].ready) {
            updateButtons();
            updateSliders();

            scheduledTasks[TASK_BUTTON_POLLING].ready = false;
        }

        if (scheduledTasks[TASK_DISPLAY].ready) {
            displayMeanVal(rawMeasuredAltitude, measuredAltitude, state);
            displayYaw(measuredYaw);
            displayRotorPWM(getPWMDuty(ROTOR_MAIN), getPWMDuty(ROTOR_TAIL));

            scheduledTasks[TASK_DISPLAY].ready = false;
        }

        if (scheduledTasks[TASK_UART].ready) {
            displayUART(measuredAltitude, measuredYaw, desiredAltitude,
                        desiredYaw, getHelicopterMode(), getYawRefAngle());

            scheduledTasks[TASK_UART].ready = false;
        }

        if (scheduledTasks[TASK_BUTTON_STATUS].ready) {
            button = checkWhatButton();

            switch (button) {
            case LEFT:
                desiredYaw = (desiredYaw - 15 + 360) % 360;
                break;

            case RIGHT:
                desiredYaw = (desiredYaw + 15 + 360) % 360;
                break;

            case UP:
                desiredAltitude += 10;
                if (desiredAltitude > 100) desiredAltitude = 100;
                break;

            case DOWN:
                desiredAltitude -= 10;
                if (desiredAltitude < 0) desiredAltitude = 0;
                break;
            }

            // Checking the state of the SW1 slider
            slider = checkSlider(SW1_SLIDER);

            // Ignore any changes to the slider while in landing mode
            // and the landing is not yet complete
            if (landed || getHelicopterMode() != LANDING_MODE) {
                if (slider == SLIDE_UP && getHelicopterMode() != STARTUP_MODE) {
                    setHelicopterMode(FLYING_MODE);
                } else if (slider == SLIDE_DOWN) {
                    setHelicopterMode(LANDING_MODE);
                }
            }

            scheduledTasks[TASK_BUTTON_STATUS].ready = false;
        }

        if (scheduledTasks[TASK_PID].ready) {
            rawMeasuredAltitude = getMeanVal();
            measuredAltitude = (rawLandedAltitude - rawMeasuredAltitude) * 100 / ALTITUDE_DELTA;
            rawDesiredAltitude = rawLandedAltitude - (desiredAltitude) * ALTITUDE_DELTA/100;
            rawDesiredYaw = (int32_t)(desiredYaw * 448) / 360;

            if (getYawRefAngle() != YAW_REF_NOT_FOUND) {
                measuredYaw = getYawAngle() - getYawRefAngle();
            }

            switch (getHelicopterMode()) {
            case STARTUP_MODE:
                setPWMDuty(0, ROTOR_MAIN);
                setPWMDuty(0, ROTOR_TAIL);
                break;

            case FLYING_MODE:
                landed = false; // No longer landed

                if (needFetchInitValues) {
                    rawLandedAltitude = rawMeasuredAltitude;
                    initReferenceYaw();
                    needFetchInitValues = false;
                }

                if (getYawRefAngle() != YAW_REF_NOT_FOUND) {
                    if (rawLandedAltitude != ALTITUDE_NOT_FOUND) {
                        // Main rotor
                        pidControl(rawMeasuredAltitude, rawDesiredAltitude, ALTITUDE, ROTOR_MAIN);
                    }

                    // Tail rotor
                    pidControl(measuredYaw, rawDesiredYaw, YAW, ROTOR_TAIL);
                }

                break;

            case LANDING_MODE:
                desiredAltitude = 0;
                desiredYaw = 0;

                if (landed) {
                    setPWMDuty(0, ROTOR_MAIN);
                    setPWMDuty(0, ROTOR_TAIL);
                } else {
                    pidControl(rawMeasuredAltitude, rawDesiredAltitude, ALTITUDE, ROTOR_MAIN);  // Main
                    pidControl(measuredYaw, rawDesiredYaw, YAW, ROTOR_TAIL);    // Tail

                    // Set the helicopter to landed only when the altitude is
                    // at 10% and the yaw is within 5 degrees of the reference angle
                    if (measuredAltitude <= 0 && abs(measuredYaw) < 5) {
                        landed = true;
                    }
                }
                break;
            }

            scheduledTasks[TASK_PID].ready = false;
        }
    }
}
