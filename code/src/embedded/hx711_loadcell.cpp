#ifndef NATIVE

#include "hx711_loadcell.h"
#include "constants.h"

HX711LoadCell::HX711LoadCell(int dout, int sck, float scale) : hx711(dout, sck), scale(scale)
{
}

void HX711LoadCell::update()
{
    if (newWeight)
    {
        newWeight = false;
    }

    if (hx711.readyToSend())
    {
        sumWeights += hx711.read();
        index++;
    }

    if (index == LC_NUM_SAMPLES)
    {
        newWeight = true;
        lastWeight = (float)sumWeights / (float)LC_NUM_SAMPLES;
        sumWeights = 0;
        index = 0;
    }
}

float HX711LoadCell::getWeight()
{
    return ((float)lastWeight * scale) - (float)offset;
}

bool HX711LoadCell::isNewWeight()
{
    return newWeight;
}

void HX711LoadCell::tare()
{
    offset = lastWeight;
}

void HX711LoadCell::setScale(float scale)
{
    this->scale = scale;
}

#endif