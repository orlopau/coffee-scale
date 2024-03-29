#include <cmath>

#include "loadcell.h"
#include "millis.h"

DefaultWeightSensor::~DefaultWeightSensor()
{
    delete ringBuffer;
}

DefaultWeightSensor::DefaultWeightSensor(LoadCell &loadCell)
    : loadCell(loadCell), ringBuffer(new RingBuffer<long>(1)) {}

void DefaultWeightSensor::begin()
{
    loadCell.begin();
}

void DefaultWeightSensor::update()
{
    newWeight = false;

    if (loadCell.isReady())
    {
        ringBuffer->push(loadCell.read());
        newWeight = true;

        float passedSeconds = (now() - lastWeightTime) / 1000.0;
        lastWeightTime = now();

        float deltaPerSecond = abs(ringBuffer->getRelative(0) - ringBuffer->getRelative(-1)) / passedSeconds;
        if (deltaPerSecond < deltaPerSChange)
        {
            if (numAveragingActivations < ringBuffer->capacity())
            {
                numAveragingActivations++;
            }
        }
        else
        {
            numAveragingActivations = 0;
        }
    }
}

long DefaultWeightSensor::getRawWeight()
{
    if (numAveragingActivations > 0)
    {
        // We use only the last numAveragingActivations values,
        // because we only want to average values after averaging was disabled,
        // not before, because that would lead to jumping in the weight value.
        unsigned int min;
        if (numAveragingActivations < ringBuffer->capacity())
        {
            min = numAveragingActivations;
        }
        else
        {
            min = ringBuffer->capacity();
        }

        return ringBuffer->averageLast(min);
    }
    else
    {
        return ringBuffer->getRelative(0);
    }
}

float DefaultWeightSensor::getWeight()
{
    return (getRawWeight() - offset) * scale;
}

float DefaultWeightSensor::getLastWeight() { return (ringBuffer->getRelative(0) - offset) * scale; }

bool DefaultWeightSensor::isNewWeight()
{
    return newWeight;
}

void DefaultWeightSensor::tare()
{
    offset = getRawWeight();
}

void DefaultWeightSensor::setScale(float scale)
{
    this->scale = scale;
}

void DefaultWeightSensor::setAutoAveraging(unsigned long deltaChange, uint8_t samples)
{
    this->deltaPerSChange = deltaChange;
    this->ringBuffer = new RingBuffer<long>(samples);
}