#ifndef RESET_BUTTON_H_
#define RESET_BUTTON_H_

/*
 * resetButton.h
 *
 *  Author: Coppy Nawaphanarat (44035285), Grant Wong, Will Archer
 */

#include <stdint.h>

// Reset configuration (active LOW) (J1-09)
#define RESET_PORT_BASE     GPIO_PORTA_BASE
#define RESET_PERIPH        SYSCTL_PERIPH_GPIOA
#define RESET_INT_PIN       GPIO_INT_PIN_6
#define RESET_PIN      GPIO_PIN_6
#define RESET_SIG_STRENGTH  GPIO_STRENGTH_4MA
#define RESET_PIN_TYPE      GPIO_PIN_TYPE_STD_WPU
#define RESET_EDGE_TYPE     GPIO_BOTH_EDGES


void initResetButton(void);

void resetButtonIntHandler(void);

#endif /* RESET_BUTTON_H_ */
