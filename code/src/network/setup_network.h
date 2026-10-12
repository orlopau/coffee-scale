#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * The scale's own network during the Wi-Fi setup: its name, its password, and
 * the Wi-Fi QR code that joins it.
 */
namespace SetupNetwork
{
    /** The same every time. WPA2 needs at least 8 characters. */
    constexpr const char *PASSWORD = "coffee-scale";

    /** "CoffeeScale-" and the MAC's last three bytes as a number, the name AutoConnect used too. */
    void name(const uint8_t mac[6], char *out, size_t size);

    /** The text of a Wi-Fi QR code that joins the network name with PASSWORD. */
    void qrPayload(const char *name, char *out, size_t size);
}
