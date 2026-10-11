#pragma once

#include <stdint.h>

namespace LoadCell
{
    void begin();
    /** The HX711's reading, signed. */
    long read();
    bool isReady();

    /**
     * The HX711's 24-bit two's complement reading as a signed number. Only the
     * low 24 bits count, so a value that is signed already stays the same.
     */
    inline int32_t toSigned(uint32_t bits)
    {
        bits &= 0xFFFFFF;
        return (bits & 0x800000) ? static_cast<int32_t>(bits) - 0x1000000 : static_cast<int32_t>(bits);
    }
};
