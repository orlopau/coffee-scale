#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * Text helpers for the log that the scale sends to the dev server.
 */
namespace NetLogText
{
    /**
     * Removes ANSI escape sequences (ESC, '[', parameter bytes, one final byte
     * in 0x40-0x7E) from text in place, because ESP_LOG's colors are noise in
     * a log file. A sequence cut off at the end of the chunk is removed up to
     * the end. Never reads past length: text isn't NUL-terminated. Returns the
     * new length and doesn't write a terminator.
     */
    size_t stripColors(char *text, size_t length);

    /**
     * Writes the notice "net_log: <count> lines dropped\n" into out, NUL-
     * terminated and truncated to size, for when the ring buffer overflowed.
     * Returns the length written without the NUL, 0 if size is 0.
     */
    size_t droppedLine(uint32_t count, char *out, size_t size);
}
