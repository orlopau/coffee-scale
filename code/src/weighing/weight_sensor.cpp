#include "weighing/weight_sensor.h"

DefaultWeightSensor::DefaultWeightSensor(SampleSource &samples) : samples(samples)
{
    readings.reserve(EXPECTED_READINGS);
}

void DefaultWeightSensor::update()
{
    readings.clear();
    Sample sample;
    while (samples.next(sample))
    {
        readings.push_back(pipeline.process(sample));
    }
}
