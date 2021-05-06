#ifndef MOTOR_H_
#define MOTOR_H_

/*
 * motor.h
 *
 * Author: Coppy Nawaphanarat (44035285), Grant Wong, Will Archer
 */

#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"

// Systick configuration
#define SYSTICK_RATE_HZ    100

// PWM configuration
#define PWM_START_RATE_HZ  250
#define PWM_RATE_STEP_HZ   50
#define PWM_RATE_MIN_HZ    50
#define PWM_RATE_MAX_HZ    400
#define PWM_START_DUTY     5
#define PWM_DUTY_STEP      5
#define PWM_DUTY_MAX       95
#define PWM_DUTY_MIN       5
#define PWM_DIVIDER_CODE   SYSCTL_PWMDIV_4
#define PWM_DIVIDER        4

//  PWM Hardware Details M0PWM7 (gen 3)
//  ---Main Rotor PWM: PC5, J4-05
#define PWM_MAIN_BASE        PWM0_BASE
#define PWM_MAIN_GEN         PWM_GEN_3
#define PWM_MAIN_OUTNUM      PWM_OUT_7
#define PWM_MAIN_OUTBIT      PWM_OUT_7_BIT
#define PWM_MAIN_PERIPH_PWM  SYSCTL_PERIPH_PWM0
#define PWM_MAIN_PERIPH_GPIO SYSCTL_PERIPH_GPIOC
#define PWM_MAIN_GPIO_BASE   GPIO_PORTC_BASE
#define PWM_MAIN_GPIO_CONFIG GPIO_PC5_M0PWM7
#define PWM_MAIN_GPIO_PIN    GPIO_PIN_5

// Tail PWM output

// pwm module
#define PWM_TAIL_BASE        PWM1_BASE
// generator
#define PWM_TAIL_GEN         PWM_GEN_2
// output number
#define PWM_TAIL_OUTNUM      PWM_OUT_5
// output bit
#define PWM_TAIL_OUTBIT      PWM_OUT_5_BIT
// which PWM to use
#define PWM_TAIL_PERIPH_PWM  SYSCTL_PERIPH_PWM1
// GPIO port
#define PWM_TAIL_PERIPH_GPIO SYSCTL_PERIPH_GPIOF
#define PWM_TAIL_GPIO_BASE   GPIO_PORTF_BASE
// GPIO port + GPIO pin
#define PWM_TAIL_GPIO_CONFIG GPIO_PF1_M1PWM5
// GPIO pin
#define PWM_TAIL_GPIO_PIN    GPIO_PIN_1

typedef enum {ROTOR_MAIN, ROTOR_TAIL} Rotor;

uint8_t getPWMDuty(Rotor rotor);
void setPWMDuty(uint32_t ui32Duty, Rotor rotor);

void initMainPWM(void);
void initTailPWM(void);
void initPWM(void);

#endif /* MOTOR_H_ */
