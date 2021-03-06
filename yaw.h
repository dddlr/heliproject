#ifndef YAW_H_
#define YAW_H_

/*
 * yaw.h
 *
 * Author:  Coppy Nawaphanarat (44035285), Grant Wong, Will Archer
 */

//*****************************************************************************
// Constants
//
// Yaw Quadrature Encoding:
// PB0 is the Channel/Phase A pin
// PB1 is the Channel/Phase B pin
//*****************************************************************************

/*
 * Yaw configuration
 */
#define YAW_QUAD_PERIPH_GPIO    SYSCTL_PERIPH_GPIOB
#define YAW_QUAD_BASE           GPIO_PORTB_BASE
#define YAW_QUAD_INT_PIN_0      GPIO_INT_PIN_0
#define YAW_QUAD_PIN_0          GPIO_PIN_0
#define YAW_QUAD_INT_PIN_1      GPIO_INT_PIN_1
#define YAW_QUAD_PIN_1          GPIO_PIN_1
#define YAW_QUAD_SIG_STRENGTH   GPIO_STRENGTH_4MA
#define YAW_QUAD_PIN_TYPE       GPIO_PIN_TYPE_STD_WPD
#define YAW_QUAD_EDGE_TYPE      GPIO_BOTH_EDGES
#define YAW_QUAD_DDR            GPIO_DIR_MODE_IN

/*
 * Yaw reference configuration
 */
#define YAW_REF_PERIPH_GPIO     SYSCTL_PERIPH_GPIOC
#define YAW_REF_BASE            GPIO_PORTC_BASE
#define YAW_REF_INT_PIN         GPIO_INT_PIN_4
#define YAW_REF_PIN             GPIO_PIN_4
#define YAW_REF_SIG_STRENGTH    GPIO_STRENGTH_4MA
#define YAW_REF_PIN_TYPE        GPIO_PIN_TYPE_STD_WPU
#define YAW_REF_EDGE_TYPE       GPIO_BOTH_EDGES
#define YAW_REF_DDR             GPIO_DIR_MODE_IN

#define YAW_TOTAL_NOTCHES       112
// Yaw angle is measured in number of notches, using
// quadrature decoding (hence multiply by 4)
#define YAW_MAX_ANGLE           (YAW_TOTAL_NOTCHES * 4)

// clockwise
#define QUAD_CW    1
// counter-clockwise
#define QUAD_CCW   -1
// no change
#define QUAD_NULL  0
// skipped a state, indicates a problem in the code
#define QUAD_ERROR 2

// The duty cycle to use for reference yaw angle
// Not used outside of yaw.c
#define YAW_REF_DUTY    45

#define YAW_REF_NOT_FOUND -1

void initYaw(void);
void readYawOutput(void);
void yawRefIntHandler(void);
void yawIntHandler(void);
void initReferenceYaw(void);
int16_t getYawRefAngle(void);
int16_t getYawAngle(void);
int8_t getYawDirection(void);

#endif /* YAW_H_ */
