#include <Arduino.h>
#include <unity.h>

#include "test_loadcell.h"
#include "test_adc_battery.h"
#include "test_display.h"

void setUp(void)
{
}

void tearDown(void)
{
}


void setup()
{
    delay(2000);
    Serial.begin(115200);

    UNITY_BEGIN();
    test_adc_battery_all();
    UNITY_END();
}

void loop()
{
}
