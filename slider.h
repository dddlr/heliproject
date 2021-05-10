#ifndef SLIDER_H_
#define SLIDER_H_

/*
 * slider.h
 *
 *  Author: Coppy Nawaphanarat (44035285), Grant Wong, Will Archer
 */

#include <stdint.h>

typedef enum {SW1_SLIDER, SW2_SLIDER, NUM_SLIDERS} switchSlider;
typedef enum {SLIDE_UP, SLIDE_DOWN, NOCHANGE} sliderState;

void initClock(void);
void initSlider(void);
void sliderIntHandler(void);
void updateSlider(void);
sliderState checkSlider(switchSlider slider);

#endif /* SLIDER_H_ */
