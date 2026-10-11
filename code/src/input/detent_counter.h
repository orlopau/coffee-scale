#pragma once

#include <stdint.h>

/**
 * Turns the encoder's counts into detents. The encoder gives 4 counts per
 * detent and rests at a multiple of 4. The detent changes once the counts are
 * 3 away from the current detent's position, just before the encoder clicks
 * in, so a bounce of one count at an edge can't make it flicker.
 */
class DetentCounter
{
public:
    static constexpr int32_t COUNTS_PER_DETENT = 4;
    /** Counts away from the current detent's position before the detent changes. */
    static constexpr int32_t THRESHOLD = 3;

    /** Takes the counts since start and returns the detent position. */
    int32_t update(int32_t counts);

private:
    int32_t detent = 0;
};
