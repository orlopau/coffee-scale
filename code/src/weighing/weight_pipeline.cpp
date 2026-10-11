#include "weighing/weight_pipeline.h"

Reading WeightPipeline::process(const Sample &sample)
{
    const float grams = sample.raw * gramsPerCount;
    const float smoothed = average.add(sample.ms, grams);

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
    if (!started)
    {
        return;
    }

    offset = average.value();
    last.net = last.grams - offset;
    last.filtered = 0;
}
