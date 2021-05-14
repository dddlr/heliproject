//*****************************************************************************
// display.c - Functions related to displaying the helicopter data
//
// Author:  Coppy Nawaphanarat, Grant Wong, Will Archer
//
//*****************************************************************************
// Description:
// Displays altitude, yaw angle, yaw direction, duty cycle, and current
// operating mode (LAND/FLY).
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "utils/ustdlib.h"
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "inc/hw_memmap.h"
#include "buttonsAPI.h"
#include "display.h"
#include "yaw.h"

//DEBUG
#include "control.h"

//********************************************************
// Constants
//********************************************************
#define SEND_STATUS_RATE_HZ     4       // Updates the uart display at 4 updates per seconds????
#define UART_INPUT_BUFFER_SIZE  50      // The size of the string for UART display
//---USB Serial comms: UART0, Rx:PA0 , Tx:PA1
#define BAUD_RATE               9600
#define UART_USB_BASE           UART0_BASE
#define UART_USB_PERIPH_UART    SYSCTL_PERIPH_UART0
#define UART_USB_PERIPH_GPIO    SYSCTL_PERIPH_GPIOA
#define UART_USB_GPIO_BASE      GPIO_PORTA_BASE
#define UART_USB_GPIO_PIN_RX    GPIO_PIN_0
#define UART_USB_GPIO_PIN_TX    GPIO_PIN_1
#define UART_USB_GPIO_PINS      UART_USB_GPIO_PIN_RX | UART_USB_GPIO_PIN_TX

//********************************************************
// Global variables
//********************************************************
static char *statusMessage;
// A boolean that checks if it is ready to send the helicopter's status
volatile bool sendStatus = false;

/**
 * Initialises display.
 */
void initDisplay(void)
{
    OLEDInitialise();
}

/**
 * Initialises UART display - 8 bit, 1 stop bit, no parity
 */
void initUART(void)
{
    statusMessage = malloc(UART_INPUT_BUFFER_SIZE * sizeof(char));
    //
    // Enable GPIO port A which is used for UART0 pins.
    //
    SysCtlPeripheralEnable(UART_USB_PERIPH_UART);
    SysCtlPeripheralEnable(UART_USB_PERIPH_GPIO);
    //
    // Select the alternate (UART) function for these pins.
    //
    GPIOPinTypeUART(UART_USB_GPIO_BASE, UART_USB_GPIO_PINS);
    GPIOPinConfigure (GPIO_PA0_U0RX);
    GPIOPinConfigure (GPIO_PA1_U0TX);

    UARTConfigSetExpClk(UART_USB_BASE, SysCtlClockGet(), BAUD_RATE,
            UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
            UART_CONFIG_PAR_NONE);
    UARTFIFOEnable(UART_USB_BASE);
    UARTEnable(UART_USB_BASE);
}

/**
 * Sending the messages to UART
 */
void UARTSend(char *statusMessage)
{
    // Loop while there are more characters to send.
    while (*statusMessage) {
        // Write the next character to the UART Tx FIFO.
        UARTCharPut(UART_USB_BASE, *statusMessage);
        statusMessage++;
    }
}

/**
 * Displays the helicopter's status message on UART
 */
void displayUART(int32_t measuredAltitude, int16_t measuredYaw, int32_t desiredAltitude, int16_t desiredYaw)
{
    //TODO: Implement Duty cycle and Mode

    measuredYaw = MAX_ANGLE_DEGS * measuredYaw / YAW_MAX_ANGLE;     // Getting the actual yaw degrees

    usprintf(statusMessage,
             "---------- \r\n Yaw: %3d [%3d] | Alt: %3d%% [%3d%%] \r\n Control: main %3d%% / tail %3d%% \r\n",
             measuredYaw, desiredYaw, measuredAltitude, desiredAltitude,
             getPWMDuty(ROTOR_MAIN), getPWMDuty(ROTOR_TAIL));

    UARTSend(statusMessage);
}

/**
 * Displays the altitude, mean ADC value (10-bit value), and sample count.
 */
void displayMeanVal(int16_t meanVal, int32_t altitudePercentage, DisplayState state)
{
    // 16 characters across the display
    // Ensure that resulting string is empty wherever text is not added
    char string[17];
    // Form a new string for the line.  The maximum width specified for the
    //  number field ensures it is displayed right justified.

    // Display the altitude percentage
    if (state == DISPLAY_ALTITUDE) {
        usnprintf(string, sizeof(string), "Alt. = %4d%%", altitudePercentage);
    // Display the mean ADC value
    } else if (state == DISPLAY_MEAN) {
        usnprintf(string, sizeof(string), "Mean = %5d", meanVal);
    } else if (state == DISPLAY_BLANK) {
        // Don't display any text.
        usnprintf(string, sizeof(string), "                ");
    }

    // Update line on display.
    OLEDStringDraw(string, 0, 0);
}

/**
 * Displays yaw angle and yaw direction.
 */
void displayYaw(int16_t yawAngle)
{
    char string[17];
    // Convert from number of notches to degrees (rounding down)
    int16_t angle = MAX_ANGLE_DEGS * yawAngle / YAW_MAX_ANGLE;
    // Change angle range to [-180 degrees, 180 degrees]
    if (angle > MAX_ANGLE_DEGS / 2) {
        angle -= MAX_ANGLE_DEGS;
    }

    usnprintf(string, sizeof(string), "Yaw = %5d", angle);
    OLEDStringDraw(string, 0, 1);
}

void displayRotorPWM(uint8_t mainPWMDuty, uint8_t tailPWMDuty)
{
    char string[17];
    usnprintf(string, sizeof(string), "MainMotor = %3d%%", mainPWMDuty);
    OLEDStringDraw(string, 0, 2);

    usnprintf(string, sizeof(string), "TailMotor = %3d%%", tailPWMDuty);
    OLEDStringDraw(string, 0, 3);
}
