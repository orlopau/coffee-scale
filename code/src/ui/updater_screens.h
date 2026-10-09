#pragma once

#include <stdint.h>
#include "canvas.h"

/**
 * Screens of the firmware updater. The updater itself only runs on the
 * device, its screens are drawn here so native tests can render them too.
 */
namespace UpdaterScreens
{
    /** A short status message, e.g. "Updating...". */
    void message(Canvas &canvas, const char *text);
    /** Running text over several lines, e.g. the Wi-Fi setup instructions. */
    void text(Canvas &canvas, const char *text);
    /** A choice between options, e.g. of the firmware language. */
    void switcher(Canvas &canvas, const char *title, uint8_t index, uint8_t count, const char *const options[]);
}
