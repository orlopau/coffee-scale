#ifndef NATIVE

#include <Arduino.h>

#include "hx711_loadcell.h"
#include "constants.h"

HX711LoadCell::HX711LoadCell(int dout, int sck, float scale) : scale(scale), sck(sck), dout(dout)
{
}

void HX711LoadCell::begin()
{
    pinMode(sck, OUTPUT);
    pinMode(dout, INPUT);
}

bool HX711LoadCell::isReady()
{
    return digitalRead(dout) == LOW;
}

long HX711LoadCell::read()
{
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&mux);

    // shift out bits
    long value = 0;
    for (int i = 0; i < 24; i++)
    {
        digitalWrite(sck, HIGH);
        delayMicroseconds(1);
        value = value << 1;
        if (digitalRead(dout))
        {
            value++;
        }
        digitalWrite(sck, LOW);
        delayMicroseconds(1);
    }

    // send gain of 128 (25 clock pulses in sum)
    digitalWrite(sck, HIGH);
    delayMicroseconds(1);
    digitalWrite(sck, LOW);
    delayMicroseconds(1);

    portEXIT_CRITICAL(&mux);
    return value;
}

void HX711LoadCell::update()
{
    newWeight = false;

    if (isReady())
    {
        lastWeight = read();
        newWeight = true;
    }
}

float HX711LoadCell::getWeight()
{
    return (lastWeight - offset) * scale;
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