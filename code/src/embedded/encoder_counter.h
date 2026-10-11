#ifndef NATIVE

#pragma once

#include <stdint.h>

/**
 * Counts the encoder's quadrature signal in the ESP32's pulse counter, 4 counts
 * per detent, without an interrupt. The 16-bit hardware counter is moved into
 * a 32-bit base before it can overflow.
 */
class EncoderCounter
{
public:
    /** Moves the hardware counter into the base once it gets this far from 0. */
    static constexpr int16_t CLEAR_AT = 16384;

    /** Sets up and starts the pulse counter. False, and logged, on an error. */
    bool begin();
    /** Counts since begin(), positive clockwise. 0 if begin() failed. */
    int32_t read();

private:
    bool running = false;
    int32_t base = 0;
};

#endif
