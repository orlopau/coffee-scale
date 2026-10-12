#pragma once

#include <stdint.h>
#include <vector>

#include "weighing/adaptive_average.h"
#include "weighing/flow_estimator.h"
#include "weighing/sample.h"
#include "weighing/weight_source.h"
#include "weighing/zero_tracker.h"

/**
 * Turns the samples of the load cell into readings, stage by stage:
 * calibration, filter, flow, zero tracking and tare.
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
    /**
     * Tares to a weight in calibrated grams, like Reading::grams, instead of
     * the averaged one, e.g. the weight from before a lift. Does nothing
     * before the first sample.
     */
    void tareTo(float grams);

    /** A new calibration. The tare still holds, filter and flow start over. */
    void setGramsPerCount(float gramsPerCount);
    /** See AdaptiveAverage::configure(). */
    void setAutoAveraging(float gramsPerSecond, uint8_t samples) { average.configure(gramsPerSecond, samples); }
    /** Grams around zero that zero tracking pulls back to zero, 0 (the default) turns it off. */
    void setZeroTracking(float windowGrams) { zeroTracker.setWindow(windowGrams); }
    /**
     * While held, zero tracking leaves the zero alone, e.g. while a mode
     * measures. Set once per loop, before update(). A mode that starts
     * measuring in a loop is held from the next one on; the readings of that
     * one loop move the zero by 0.002 g at most.
     */
    void holdZero(bool held) { zeroTracker.hold(held); }

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
    ZeroTracker zeroTracker;

    bool started = false;
    Reading last = {};
};
