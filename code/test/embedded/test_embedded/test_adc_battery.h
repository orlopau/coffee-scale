#include <unity.h>
#include <Arduino.h>

#include "embedded/adc_battery.h"
#include "embedded/constants.h"

#include "confirm.h"

ADCBattery battery(PIN_BAT_ADC, PIN_BAT_CHARGE_STATUS, BAT_V_MULTIPLIER, 0, BAT_V_MIN, BAT_V_MAX);

void test_adc_battery(void)
{
    float voltage = battery.getVoltage();
    TEST_ASSERT_GREATER_OR_EQUAL(0, voltage);
    Serial.printf("voltage: %f\n", voltage);
    wait_confirm();
    Serial.println("----------------------");
}

void test_adc_battery_is_charging()
{
    Serial.println("Remove power from device!");
    wait_confirm();
    TEST_ASSERT_FALSE(battery.isCharging());
    Serial.println("Connect power to device!");
    wait_confirm();
    TEST_ASSERT_TRUE(battery.isCharging());
    Serial.println("----------------------");
}

void test_adc_battery_all()
{
    RUN_TEST(test_adc_battery);
}
