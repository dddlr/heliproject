#ifndef SLIDER_H_
#define SLIDER_H_

/*
 * slider.h
 *
 *  Author: Coppy Nawaphanarat (44035285), Grant Wong, Will Archer
 */

#include <stdint.h>

// Slider MAIN base
#define SLIDER_PORT_BASE  GPIO_PORTA_BASE

// SW1: PA7 (J1-10)
#define SW1_PERIPH        SYSCTL_PERIPH_GPIOA   //At A-Pin
#define SW1_PORT_PIN      GPIO_PIN_7

// SW2: PA6 (J1-09)
#define SW2_PERIPH        SYSCTL_PERIPH_GPIOA
#define SW2_PORT_PIN      GPIO_PIN_6

typedef enum {SW1_SLIDER, SW2_SLIDER, NUM_SLIDERS} switchSlider;
typedef enum {SLIDE_UP, SLIDE_DOWN, NUM_SLIDER_STATES} sliderState;

void initSlider(void);
void updateSliders(void);
sliderState checkSlider(switchSlider slider);

#endif /* SLIDER_H_ */
