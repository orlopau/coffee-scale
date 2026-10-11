#pragma once

#include <stdint.h>
#include <vector>

#include "weighing/sample.h"
#include "weighing/weight_pipeline.h"

class WeightSensor
{
public:
    virtual ~WeightSensor(){};
    virtual void begin(){};
    /** Takes the samples that arrived since the last call and turns them into readings. */
    virtual void update(){};
    /** All readings since the last update(), oldest first. Empty if there were none. */
    virtual const std::vector<Reading> &newReadings() = 0;
    /**
     * @brief The weight in grams to show: tared, and averaged while it holds still.
     */
    virtual float getWeight() = 0;
    virtual void tare() = 0;
    virtual void setScale(float gramsPerCount) = 0;
    /**
     * @brief Set auto averaging for the sensor, see AdaptiveAverage.
     *
     * @param gramsPerSecond change per second under which the weight is
     * averaged, 0 to disable auto averaging
     * @param samples the most samples to average
     */
    virtual void setAutoAveraging(float gramsPerSecond, uint8_t samples){};
};

/** The weight from the samples of a SampleSource, through a WeightPipeline. */
class DefaultWeightSensor : public WeightSensor
{
public:
    /** At the HX711's 10 samples per second, more never wait between two loops. */
    static constexpr size_t EXPECTED_READINGS = 32;

    explicit DefaultWeightSensor(SampleSource &samples);
    void update() override;
    const std::vector<Reading> &newReadings() override { return readings; }
    float getWeight() override { return pipeline.latest().filtered; }
    void tare() override { pipeline.tare(); }
    void setScale(float gramsPerCount) override { pipeline.setGramsPerCount(gramsPerCount); }
    void setAutoAveraging(float gramsPerSecond, uint8_t samples) override
    {
        pipeline.setAutoAveraging(gramsPerSecond, samples);
    }

private:
    SampleSource &samples;
    WeightPipeline pipeline;
    std::vector<Reading> readings;
};
