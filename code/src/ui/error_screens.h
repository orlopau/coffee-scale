#pragma once

#include "canvas.h"

/**
 * Screens of faults the scale can't fix itself. Drawn here so native tests
 * can render them too.
 */
namespace ErrorScreens
{
    /**
     * The calibration couldn't be read from flash. errorName is the ESP
     * error's name, e.g. from esp_err_to_name(). A leading "ESP_ERR_NVS_" is
     * left out, so the name fits the display.
     */
    void settingsUnreadable(Canvas &canvas, const char *errorName);
}
