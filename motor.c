//*****************************************************************************
// motor.c - Generates PWM signals for the main rotor and the tail rotor.
//
// Author:  Coppy Nawaphanarat, Grant Wong, Will Archer
//
//*****************************************************************************
// Description:
// TODO
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "motor.h"

static uint8_t mainPWMDuty = PWM_START_DUTY;
static uint8_t tailPWMDuty = PWM_START_DUTY;

uint8_t getPWMDuty(Rotor rotor)
{
    if (rotor == ROTOR_MAIN) {
        return mainPWMDuty;
    } else {
        return tailPWMDuty;
    }
}

void setPWMDuty(uint32_t duty, Rotor rotor)
{
    uint32_t period = SysCtlClockGet() / PWM_DIVIDER / PWM_START_RATE_HZ;

    if (rotor == ROTOR_MAIN) {
        mainPWMDuty = duty;
         PWMPulseWidthSet(PWM_MAIN_BASE, PWM_MAIN_OUTNUM,
              period * duty / 100);
//        PWMPulseWidthSet(PWM_MAIN_BASE, PWM_MAIN_OUTNUM, period * 2 / 100);
    } else {
        tailPWMDuty = duty;
        PWMPulseWidthSet(PWM_TAIL_BASE, PWM_TAIL_OUTNUM,
            period * duty / 100);
    }
}

void initMainPWM(void)
{
    uint32_t period = SysCtlClockGet() / PWM_DIVIDER / PWM_START_RATE_HZ;

    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_PWM);
    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_GPIO);

    GPIOPinConfigure(PWM_MAIN_GPIO_CONFIG);

    GPIOPinTypePWM(PWM_MAIN_GPIO_BASE, PWM_MAIN_GPIO_PIN);

    PWMGenConfigure(PWM_MAIN_BASE, PWM_MAIN_GEN,
                    PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

    PWMGenPeriodSet(PWM_MAIN_BASE, PWM_MAIN_GEN, period);

    // Set the initial PWM parameters
    setPWMDuty(PWM_START_DUTY, ROTOR_MAIN);

    PWMGenEnable(PWM_MAIN_BASE, PWM_MAIN_GEN);

    // Disable the output.  Repeat this call with 'true' to turn O/P on.
    PWMOutputState(PWM_MAIN_BASE, PWM_MAIN_OUTBIT, false);
}

void initTailPWM(void)
{
    uint32_t period = SysCtlClockGet() / PWM_DIVIDER / PWM_START_RATE_HZ;

    SysCtlPeripheralEnable(PWM_TAIL_PERIPH_PWM);
    SysCtlPeripheralEnable(PWM_TAIL_PERIPH_GPIO);

    GPIOPinConfigure(PWM_TAIL_GPIO_CONFIG);

    GPIOPinTypePWM(PWM_TAIL_GPIO_BASE, PWM_TAIL_GPIO_PIN);

    PWMGenConfigure(PWM_TAIL_BASE, PWM_TAIL_GEN, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

    PWMGenPeriodSet(PWM_TAIL_BASE, PWM_TAIL_GEN, period);

    setPWMDuty(PWM_START_DUTY, ROTOR_TAIL);

    PWMGenEnable(PWM_TAIL_BASE, PWM_TAIL_GEN);
    PWMOutputState(PWM_TAIL_BASE, PWM_TAIL_OUTBIT, false);
}

void initPWM(void)
{
    initMainPWM();
    initTailPWM();
    // Set the PWM clock rate (using the prescaler)
    SysCtlPWMClockSet(PWM_DIVIDER_CODE);

    // Initialisation is complete, so turn on the output.
    PWMOutputState(PWM_MAIN_BASE, PWM_MAIN_OUTBIT, true);
    PWMOutputState(PWM_TAIL_BASE, PWM_TAIL_OUTBIT, true);
}
