//*****************************************************************************
// display.c - Functions related to displaying the helicopter data
//
// Author:  Coppy Nawaphanarat, Grant Wong, Will Archer
//
//*****************************************************************************
// Description:
// Displays altitude, yaw angle, and yaw direction.
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "utils/ustdlib.h"
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "display.h"
#include "yaw.h"

/**
 * Initialises display.
 */
void initDisplay(void)
{
    OLEDInitialise();
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
    OLEDStringDraw(string, 0, 1);
}

/**
 * Displays yaw angle and yaw direction.
 */
void displayYaw(int16_t yawAngle, int8_t yawDirection)
{
    char string[17];
    // Note integer division below loses a bit of accuracy and always rounds down
    // TODO: change angle range to [-180 degrees, 180 degrees]
    usnprintf(string, sizeof(string), "YawAngle = %5d", MAX_ANGLE_DEGS*yawAngle/YAW_MAX_ANGLE);
    OLEDStringDraw(string, 0, 3);

    if (yawDirection == QUAD_CW) {
        OLEDStringDraw("Yaw ClockWise   ", 0, 0);
    } else if (yawDirection == QUAD_CCW) {
        OLEDStringDraw("Yaw CntClockwise", 0, 0);
    } else if (yawDirection == QUAD_NULL) {
        OLEDStringDraw("Yaw No Change   ", 0, 0);
    } else {
        // indicates a yaw measurement interrupt has been skipped - this
        // should never happen
        OLEDStringDraw("Yaw ERROR ERROR ", 0, 0);
    }
}
