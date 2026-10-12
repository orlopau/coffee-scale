#pragma once

#include <stdint.h>

/**
 * Decides when a newly installed firmware counts as working: once the main
 * loop has run for a while and the load cell has delivered readings. Until
 * then, a reboot rolls the scale back to the firmware before, see
 * embedded/network/firmware_check.h.
 */
class FirmwareProbation
{
public:
    /** How long the main loop has to run, in ms. */
    static constexpr uint32_t RUN_MS = 2000;

    /**
     * Once per loop. True exactly once: in the first loop at least RUN_MS
     * after the first loop, once readings arrived in any loop.
     */
    bool update(uint32_t nowMs, bool readingsArrived);

private:
    bool started = false;
    bool sawReadings = false;
    bool done = false;
    uint32_t startMs = 0;
};
