#pragma once

#include <Arduino.h>

inline void wait_confirm()
{
    Serial.println("Press y to confirm...");
    while (true)
    {
        while (Serial.available() == 0)
        {
            delay(100);
        }
        if (Serial.read() == 'y')
        {
            return;
        }
    }
}