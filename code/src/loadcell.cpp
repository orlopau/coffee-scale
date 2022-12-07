#include "loadcell.h"

DefaultWeightSensor::DefaultWeightSensor(LoadCell &loadCell) : loadCell(loadCell) {}

void DefaultWeightSensor::begin()
{
    loadCell.begin();
}

void DefaultWeightSensor::update()
{
    newWeight = false;

    if (loadCell.isReady())
    {
        lastWeight = loadCell.read();
        newWeight = true;
    }
}

float DefaultWeightSensor::getWeight()
{
    return (lastWeight - offset) * scale;
}

bool DefaultWeightSensor::isNewWeight()
{
    return newWeight;
}

void DefaultWeightSensor::tare()
{
    offset = lastWeight;
}

void DefaultWeightSensor::setScale(float scale)
{
    this->scale = scale;
}