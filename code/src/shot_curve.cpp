#include "shot_curve.h"

#include <math.h>

static uint16_t toTenths(float grams)
{
    const float tenths = roundf(grams * 10);
    if (tenths <= 0)
    {
        return 0;
    }
    return tenths >= UINT16_MAX ? UINT16_MAX : (uint16_t)tenths;
}

void ShotCurve::reset()
{
    count = 0;
    lastTenths = 0;
    recentNext = 0;
    recentCount = 0;
    duration = 0;
    ended = false;
}

void ShotCurve::add(uint32_t timeMs, float grams)
{
    const uint16_t tenths = toTenths(grams);
    // the points before this reading take the one before it
    while (count < MAX_POINTS && count * INTERVAL_MS <= timeMs)
    {
        points[count] = count * INTERVAL_MS == timeMs ? tenths : lastTenths;
        count++;
    }
    lastTenths = tenths;

    recent[recentNext] = {timeMs, tenths};
    recentNext = (recentNext + 1) % RECENT_READINGS;
    if (recentCount < RECENT_READINGS)
    {
        recentCount++;
    }
}

void ShotCurve::finish(uint32_t durationMs)
{
    size_t end = durationMs / INTERVAL_MS + 1;
    if (end > MAX_POINTS)
    {
        end = MAX_POINTS;
    }
    if (count > end)
    {
        count = end;
    }
    // ended after the last reading, e.g. by a click
    while (count < end)
    {
        points[count++] = lastTenths;
    }
    // the yield: the newest reading at or before the end, also between two points or past 90 s
    for (size_t i = 1; i <= recentCount; i++)
    {
        const Recent &reading = recent[(recentNext + RECENT_READINGS - i) % RECENT_READINGS];
        if (reading.ms <= durationMs)
        {
            points[count - 1] = reading.tenths;
            break;
        }
    }
    duration = durationMs;
    ended = true;
}

bool ShotCurve::isFinished() const
{
    return ended && duration >= MIN_DURATION_MS && yieldGrams() >= MIN_YIELD_GRAMS;
}
