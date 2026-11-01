#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * A shot's net weight over its time, one point every INTERVAL_MS from its
 * start at t = 0. Each point is the weight of the newest reading at or before
 * it, 0 before the first; the last one is the weight at the end, the yield,
 * even past 90 s. finish() makes it final at the shot's end: what the
 * finished-shot screen shows, and later the shot log stores.
 */
class ShotCurve
{
public:
    static constexpr uint32_t INTERVAL_MS = 200;
    /** 90 s. A longer shot's curve ends there, its time goes on. */
    static constexpr size_t MAX_POINTS = 450;
    /** Shorter or lighter shots aren't finished, e.g. two clicks in a row. */
    static constexpr uint32_t MIN_DURATION_MS = 5000;
    static constexpr float MIN_YIELD_GRAMS = 1.0f;
    /**
     * Readings kept for the weight at the end, which can lie between two
     * points, or come a while back when the pump stop is noticed late.
     */
    static constexpr size_t RECENT_READINGS = 32;

    /** Empty and not finished, for a new shot. */
    void reset();
    /** A reading timeMs into the shot, after the one before, with its net weight. */
    void add(uint32_t timeMs, float grams);
    /**
     * The shot ended durationMs after its start: points after it are dropped,
     * points up to it take the last reading's weight, and the last point the
     * newest reading at or before the end.
     */
    void finish(uint32_t durationMs);

    /** Ended, at least MIN_DURATION_MS long and MIN_YIELD_GRAMS heavy. */
    bool isFinished() const;
    uint32_t durationMs() const { return duration; }
    /** The weight at the end, the last point. 0 without points. */
    float yieldGrams() const { return count > 0 ? gramsAt(count - 1) : 0; }
    size_t size() const { return count; }
    /** Point i, at i × INTERVAL_MS into the shot. */
    float gramsAt(size_t i) const { return points[i] / 10.0f; }

private:
    /** In 0.1 g. */
    uint16_t points[MAX_POINTS] = {};
    size_t count = 0;
    /** The newest reading's weight, in 0.1 g. */
    uint16_t lastTenths = 0;

    struct Recent
    {
        uint32_t ms;
        uint16_t tenths;
    };
    /** A ring of the newest readings, recentCount of them up to recentNext. */
    Recent recent[RECENT_READINGS] = {};
    size_t recentNext = 0;
    size_t recentCount = 0;
    uint32_t duration = 0;
    bool ended = false;
};
