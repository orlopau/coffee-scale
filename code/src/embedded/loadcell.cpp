#ifndef NATIVE

#include <Arduino.h>

#include "constants.h"
#include "loadcell.h"

namespace LoadCell
{
    void begin()
    {
        pinMode(PIN_HX711_SCK, OUTPUT);
        pinMode(PIN_HX711_DAT, INPUT);
    }

    bool isReady() { return digitalRead(PIN_HX711_DAT) == LOW; }

    long read()
    {
        portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
        portENTER_CRITICAL(&mux);

        // shift out bits
        long value = 0;
        for (int i = 0; i < 24; i++)
        {
            digitalWrite(PIN_HX711_SCK, HIGH);
            delayMicroseconds(1);
            value = value << 1;
            if (digitalRead(PIN_HX711_DAT))
            {
                value++;
            }
            digitalWrite(PIN_HX711_SCK, LOW);
            delayMicroseconds(1);
        }

        // send gain of 128 (25 clock pulses in sum)
        digitalWrite(PIN_HX711_SCK, HIGH);
        delayMicroseconds(1);
        digitalWrite(PIN_HX711_SCK, LOW);
        delayMicroseconds(1);

        portEXIT_CRITICAL(&mux);
        return value;
    }
}

#endif