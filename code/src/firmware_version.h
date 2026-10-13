#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * Firmware versions as the release tags have them, "v1.10.0" in
 * "firmware-v1.10.0", and as FIRMWARE_VERSION holds them.
 */
namespace FirmwareVersion
{
    /** Room for a version's text with its NUL, "v65535.65535.65535" doesn't need more. */
    constexpr size_t SIZE = 20;

    struct Parts
    {
        uint16_t major, minor, patch;
    };

    /** "v1.10.0" or "1.10.0" and nothing else. False if unreadable, out stays as it was. */
    bool parse(const char *text, Parts &out);

    /** True only if both are readable and candidate is the higher version, by number. */
    bool isNewer(const char *candidate, const char *running);

    /**
     * The version in a release download's URL, e.g. "v1.10.0" from
     * ".../download/firmware-v1.10.0/firmware_en.bin", absolute or not.
     * False, and out empty, if there is none, it's unreadable, or it
     * doesn't fit into size.
     */
    bool fromRedirect(const char *location, char *out, size_t size);
}
