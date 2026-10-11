#include "weighing/weight_pipeline.h"

void WeightPipeline::setGramsPerCount(float gramsPerCount)
{
    this->gramsPerCount = gramsPerCount;
    // they hold grams of the old calibration
    average.reset();
    flow.reset();
}

Reading WeightPipeline::process(const Sample &sample)
{
    const float grams = sample.raw * gramsPerCount;
    const float smoothed = average.add(sample.ms, grams);
    const float offset = offsetCounts * gramsPerCount;

    last = {
        sample.ms,
        sample.raw,
        grams,
        grams - offset,
        smoothed - offset,
        // from grams, so taring doesn't look like a flow
        flow.add(sample.ms, grams),
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
    last.net = (last.raw - offsetCounts) * gramsPerCount;
    last.filtered = 0;
}
