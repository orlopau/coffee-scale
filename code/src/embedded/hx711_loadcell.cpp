#ifndef NATIVE

#include "hx711_loadcell.h"
#include "constants.h"

HX711LoadCell::HX711LoadCell(int dout, int sck, float scale) : dout(dout), sck(sck)
{
    hx711.set_scale(scale);
}

void HX711LoadCell::begin()
{
    hx711.begin(dout, sck);
}

void HX711LoadCell::update()
{
    if (newWeight)
    {
        newWeight = false;
    }

    if (hx711.is_ready())
    {
        sumWeights += hx711.read();
        index++;
    }

    if (index = LC_NUM_SAMPLES)
    {
        newWeight = true;
        weight = sumWeights / LC_NUM_SAMPLES;
        sumWeights = 0;
        index = 0;
    }
}

float HX711LoadCell::getWeight()
{
    return (weight - hx711.get_offset()) / hx711.get_scale();
}

bool HX711LoadCell::isNewWeight()
{
    return newWeight;
}

void HX711LoadCell::tare()
{
    hx711.set_offset(weight);
}

void HX711LoadCell::setScale(float scale)
{
    hx711.set_scale(scale);
}

#endif