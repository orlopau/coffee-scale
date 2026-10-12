#pragma once

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <vector>

/**
 * What the setup page sends and receives, apart from the HTTP server so it
 * can be tested natively.
 */
namespace SetupWeb
{
    /** A network the scale sees. */
    struct Network
    {
        std::string ssid;
        int8_t rssi;
        bool secure;
    };

    /**
     * The field key of a form body (application/x-www-form-urlencoded, the
     * first length bytes) into value, decoded: '+' is a space, %XX a byte.
     * False if the field is missing, its encoding is broken, or it is longer
     * than maxLength bytes.
     */
    bool formField(const char *body, size_t length, const char *key, std::string &value, size_t maxLength);

    /**
     * The networks as JSON for the page, strongest first:
     * [{"ssid":"Home","rssi":-52,"secure":true},...]. Networks without a name (hidden)
     * are left out, a name seen more than once (e.g. a mesh) is listed once.
     */
    std::string networksJson(std::vector<Network> networks);

    /** text as a JSON string, quoted. Bytes from 0x80 on stay as they are. */
    std::string jsonString(const std::string &text);
}
