#include "detectors/shot_detector.h"

#include <math.h>

bool ShotDetector::update(const Reading &reading)
{
    if (hasLast && reading.ms - lastMs > MAX_GAP_MS)
    {
        reset();
    }
    hasLast = true;
    lastMs = reading.ms;

    if (rising)
    {
        // a bump coming back: still armed on the same baseline, the reading counts as one outside a rise
        if (reading.grams > baseline + BACK_GRAMS)
        {
            return addRise(reading);
        }
        rising = false;
        risen.clear();
        beforeCount = 0;
        beforeNext = 0;
    }
    if (armed && reading.grams > baseline + RISE_GRAMS)
    {
        beginRise(reading);
        return false;
    }
    addSteady(reading);
    before[beforeNext] = reading;
    beforeNext = (beforeNext + 1) % LOOKBACK_READINGS;
    if (beforeCount < LOOKBACK_READINGS)
    {
        beforeCount++;
    }
    return false;
}

void ShotDetector::beginRise(const Reading &reading)
{
    rising = true;
    risen.clear();
    // back from the newest reading before the rise, to the last one at the baseline
    int drops = 0;
    while (drops < beforeCount)
    {
        const int index = (beforeNext - 1 - drops + LOOKBACK_READINGS) % LOOKBACK_READINGS;
        if (before[index].grams <= baseline + DROP_GRAMS)
        {
            break;
        }
        drops++;
    }
    for (int i = drops; i > 0; i--)
    {
        risen.push_back(before[(beforeNext - i + LOOKBACK_READINGS) % LOOKBACK_READINGS]);
    }
    risen.push_back(reading);
}

void ShotDetector::addSteady(const Reading &reading)
{
    recent[recentNext] = reading;
    recentNext = (recentNext + 1) % STEADY_READINGS;
    if (recentCount < STEADY_READINGS)
    {
        recentCount++;
    }
    if (recentCount < STEADY_READINGS)
    {
        return;
    }

    float lowest = recent[0].grams, highest = recent[0].grams, grams = 0, net = 0;
    for (const Reading &r : recent)
    {
        if (fabsf(r.flow) >= STEADY_FLOW)
        {
            return;
        }
        lowest = fminf(lowest, r.grams);
        highest = fmaxf(highest, r.grams);
        grams += r.grams;
        net += r.net;
    }
    if (highest - lowest >= STEADY_GRAMS)
    {
        return;
    }
    // the baseline only moves with a new steady stretch, and stays when the stretch ends
    baseline = grams / STEADY_READINGS;
    armed = fabsf(net / STEADY_READINGS) <= ARM_GRAMS;
}

bool ShotDetector::addRise(const Reading &reading)
{
    const uint32_t elapsed = reading.ms - risen.front().ms;
    // it could be anything by now
    if (elapsed > MAX_RISE_MS || risen.size() >= MAX_RISE_READINGS)
    {
        disarm();
        return false;
    }
    risen.push_back(reading);
    if (elapsed < CONFIRM_MS)
    {
        return false;
    }

    const float meanFlow = (reading.grams - baseline) * 1000 / elapsed;
    const float minFlow = expectedFlow * MIN_FLOW_FACTOR;
    const float maxFlow = expectedFlow * MAX_FLOW_FACTOR;
    // poured, it doesn't get slower like a shot that starts slowly gets faster
    if (meanFlow > maxFlow)
    {
        disarm();
        return false;
    }
    // still flowing, so a step that stays isn't a shot
    if (meanFlow >= minFlow && reading.flow >= minFlow)
    {
        disarm();
        return true;
    }
    return false;
}

void ShotDetector::disarm()
{
    armed = false;
    rising = false;
    recentCount = 0;
    recentNext = 0;
    beforeCount = 0;
    beforeNext = 0;
}

void ShotDetector::reset()
{
    disarm();
    risen.clear();
    hasLast = false;
}
