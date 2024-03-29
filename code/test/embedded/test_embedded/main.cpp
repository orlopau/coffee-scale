#include <Arduino.h>
#include <unity.h>

#include "test_loadcell.h"
#include "test_adc_battery.h"
#include "test_display.h"
#include "test_encoder.h"

void setUp(void)
{
}

void tearDown(void)
{
    Serial.println("--------------------");
}


void setup()
{
    Serial.begin(115200);

    UNITY_BEGIN();
    test_encoder_all();
    test_adc_battery_all();
    test_loadcell_all();
    test_display_all();
    UNITY_END();
}

void loop()
{
}
