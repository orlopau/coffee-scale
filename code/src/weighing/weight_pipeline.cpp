#include "weighing/weight_pipeline.h"

WeightPipeline::WeightPipeline(SampleSource &samples) : samples(samples) { readings.reserve(EXPECTED_READINGS); }

void WeightPipeline::update()
{
    readings.clear();
    Sample sample;
    while (samples.next(sample))
    {
        readings.push_back(process(sample));
    }
}

void WeightPipeline::setGramsPerCount(float gramsPerCount)
{
    this->gramsPerCount = gramsPerCount;
    // they hold grams of the old calibration
    average.reset();
    flow.reset();
    zeroTracker.reset();
}

Reading WeightPipeline::process(const Sample &sample)
{
    const float grams = sample.raw * gramsPerCount;
    const float smoothed = average.add(sample.ms, grams);
    // from grams, so taring doesn't look like a flow
    const float gramsPerSecond = flow.add(sample.ms, grams);
    if (gramsPerCount != 0)
    {
        offsetCounts += zeroTracker.track(sample.ms, smoothed - offsetCounts * gramsPerCount, gramsPerSecond) / gramsPerCount;
    }
    const float offset = offsetCounts * gramsPerCount;

    last = {
        sample.ms,
        sample.raw,
        grams,
        grams - offset,
        smoothed - offset,
        gramsPerSecond,
    };
    started = true;
    return last;
}

void WeightPipeline::tare()
{
    if (!started || gramsPerCount == 0)
    {
        return;
    }

    // right after a new calibration the filter is empty, the last sample is all there is
    offsetCounts = average.isEmpty() ? last.raw : average.value() / gramsPerCount;
    zeroTracker.reset();
    last.net = (last.raw - offsetCounts) * gramsPerCount;
    last.filtered = 0;
}

void WeightPipeline::tareTo(float grams)
{
    if (!started || gramsPerCount == 0)
    {
        return;
    }

    offsetCounts = grams / gramsPerCount;
    zeroTracker.reset();
    last.net = last.grams - grams;
    last.filtered = (average.isEmpty() ? last.grams : average.value()) - grams;
}
