#include <string.h>

#include "firmware_version.h"

namespace FirmwareVersion
{
    // where a release's tag sits in its download URL
    static const char *const RELEASE_TAG = "/download/firmware-";
    // a uint16_t has at most 5 digits
    static const int MAX_DIGITS = 5;

    /** Reads a number at text and moves text past it. */
    static bool readNumber(const char *&text, uint16_t &out)
    {
        uint32_t value = 0;
        int digits = 0;
        while (*text >= '0' && *text <= '9')
        {
            if (++digits > MAX_DIGITS)
            {
                return false;
            }
            value = value * 10 + (*text - '0');
            text++;
        }
        if (digits == 0 || value > UINT16_MAX)
        {
            return false;
        }
        out = (uint16_t)value;
        return true;
    }

    /** Reads c at text and moves text past it. */
    static bool expect(const char *&text, char c)
    {
        if (*text != c)
        {
            return false;
        }
        text++;
        return true;
    }

    bool parse(const char *text, Parts &out)
    {
        if (text == nullptr)
        {
            return false;
        }
        if (*text == 'v')
        {
            text++;
        }
        Parts parts;
        if (!readNumber(text, parts.major) || !expect(text, '.') || !readNumber(text, parts.minor) ||
            !expect(text, '.') || !readNumber(text, parts.patch) || *text != '\0')
        {
            return false;
        }
        out = parts;
        return true;
    }

    bool isNewer(const char *candidate, const char *running)
    {
        Parts a, b;
        if (!parse(candidate, a) || !parse(running, b))
        {
            return false;
        }
        if (a.major != b.major)
        {
            return a.major > b.major;
        }
        if (a.minor != b.minor)
        {
            return a.minor > b.minor;
        }
        return a.patch > b.patch;
    }

    bool fromRedirect(const char *location, char *out, size_t size)
    {
        if (size > 0)
        {
            out[0] = '\0';
        }
        if (location == nullptr || size == 0)
        {
            return false;
        }
        const char *start = strstr(location, RELEASE_TAG);
        if (start == nullptr)
        {
            return false;
        }
        start += strlen(RELEASE_TAG);
        const char *end = strchr(start, '/');
        if (end == nullptr || end == start || (size_t)(end - start) >= size)
        {
            return false;
        }
        memcpy(out, start, end - start);
        out[end - start] = '\0';
        Parts parts;
        if (!parse(out, parts))
        {
            out[0] = '\0';
            return false;
        }
        return true;
    }
}
