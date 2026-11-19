#pragma once

#include <stdint.h>

/**
 * How long the grinder's task waits between connection attempts to a network it cannot reach:
 * connecting and scanning draw far more power than an idle connection.
 */
namespace ReconnectBackoff
{
    static const uint32_t FIRST_MS = 2000;
    static const uint32_t MAX_MS = 60000;

    /** The wait after a failure that followed a wait of waitMs: doubled, but at most MAX_MS. */
    inline uint32_t next(uint32_t waitMs)
    {
        return waitMs >= MAX_MS / 2 ? MAX_MS : waitMs * 2;
    }

    /** Whether the wait since a failure at failedAt is over at time at; the signed difference survives the clock's wrap. */
    inline bool over(uint32_t failedAt, uint32_t waitMs, uint32_t at)
    {
        return static_cast<int32_t>(at - failedAt) >= static_cast<int32_t>(waitMs);
    }
}
