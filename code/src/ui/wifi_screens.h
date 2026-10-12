#pragma once

#include <stdint.h>

#include "canvas.h"

/**
 * Screens of the Wi-Fi setup. The setup itself only runs on the device, its
 * screens are drawn here so native tests can render them too.
 */
namespace WifiScreens
{
    /** The Wi-Fi QR code on the left; the network's name and password on the right, for typing them in by hand. */
    void join(Canvas &canvas, const char *qrPayload, const char *name, const char *password);
    /** A status of the setup, e.g. "Connecting...", word-wrapped. */
    void status(Canvas &canvas, const char *text);
    /** The first start's question whether to set up Wi-Fi, index 0 is "Yes", 1 "Later". */
    void ask(Canvas &canvas, uint8_t index);
}
