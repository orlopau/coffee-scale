#include <math.h>

#include "weighing/zero_tracker.h"

float ZeroTracker::track(uint32_t ms, float grams, float flow)
{
    // unsigned, so this stays right when the clock wraps after 49 days
    const uint32_t elapsedMs = ms - lastMs;
    const bool first = !hasLast;
    // also while held, so the time held isn't made up for afterwards
    hasLast = true;
    lastMs = ms;

    if (first || held || fabsf(grams) > window || fabsf(flow) > STEADY_GRAMS_PER_SECOND)
    {
        return 0;
    }

    const float limit = GRAMS_PER_SECOND * (elapsedMs < MAX_STEP_MS ? elapsedMs : MAX_STEP_MS) / 1000.0f;
    return fmaxf(-limit, fminf(limit, grams));
}
