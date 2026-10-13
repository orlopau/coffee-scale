#pragma once

#include <stdint.h>

/**
 * The decisions of the automatic updates, see embedded/network/auto_update.h:
 * when to check for a newer firmware, which version is worth offering, and
 * when to ask about it.
 */
namespace AutoUpdatePolicy
{
    /** A check is due this many starts after the last successful one. */
    constexpr uint8_t CHECK_EVERY_STARTS = 5;

    /** The starts since the last successful check, with this one, up to 255. */
    uint8_t countStart(uint8_t starts);

    /** Whether to check at this start; starts includes this one. Never on probation. */
    bool isDue(uint8_t starts, bool onProbation);

    /** found is newer than running and isn't the version that was rolled back. Empty means none. */
    bool worthOffering(const char *found, const char *running, const char *rolledBack);

    /** Whether to ask at this start about found. Never on probation. */
    bool shouldAsk(const char *found, const char *running, const char *rolledBack, bool onProbation);
}
