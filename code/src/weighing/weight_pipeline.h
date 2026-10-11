#pragma once

#include <stdint.h>
#include <vector>

#include "weighing/adaptive_average.h"
#include "weighing/flow_estimator.h"
#include "weighing/sample.h"
#include "weighing/weight_source.h"

/**
 * Turns the samples of the load cell into readings, stage by stage:
 * calibration, filter, tare and flow.
 *
 * update() takes all samples that arrived since the loop's last pass, the
 * modes read the result through WeightSource.
 */
class WeightPipeline : public WeightSource
{
public:
    /** At the HX711's 10 samples per second, more never wait between two loops. */
    static constexpr size_t EXPECTED_READINGS = 32;

    explicit WeightPipeline(SampleSource &samples);

    /** Takes all waiting samples through the stages, into newReadings(). Call once per loop. */
    void update();
    const std::vector<Reading> &newReadings() override { return readings; }
    float getWeight() override { return last.filtered; }
    /** Zeroes the averaged weight. Does nothing before the first sample. */
    void tare() override;

    /** A new calibration. The tare still holds, filter and flow start over. */
    void setGramsPerCount(float gramsPerCount);
    /** See AdaptiveAverage::configure(). */
    void setAutoAveraging(float gramsPerSecond, uint8_t samples) { average.configure(gramsPerSecond, samples); }

    /** Takes one sample through the stages. update() does this for each waiting sample. */
    Reading process(const Sample &sample);

    bool hasReading() const { return started; }
    /** The reading of the last sample, all zero before the first. */
    const Reading &latest() const { return last; }

private:
    SampleSource &samples;
    std::vector<Reading> readings;

    float gramsPerCount = 1;
    /** In counts, so a new calibration keeps the tare. */
    float offsetCounts = 0;
    AdaptiveAverage average;
    FlowEstimator flow;

    bool started = false;
    Reading last = {};
};
