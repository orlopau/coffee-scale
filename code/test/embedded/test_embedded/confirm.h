#pragma once

#include <Arduino.h>

#define TAG "CONFIRM"

inline void wait_confirm()
{
    ESP_LOGI(TAG, "Press y to confirm...");
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