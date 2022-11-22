#include <unity.h>

#include "embedded/hx711_loadcell.h"
#include "embedded/constants.h"

void test_hx711_read(void)
{
    HX711LoadCell hx711(PIN_HX711_DAT, PIN_HX711_SCK);
    hx711.begin();
    delay(1000);
    hx711.update();
    TEST_ASSERT_TRUE(hx711.isNewWeight());
    float weight = hx711.getWeight();
    TEST_ASSERT_GREATER_OR_EQUAL(0, weight);
    Serial.printf("weight: %f\n", weight);
}

void test_loadcell_all()
{
    RUN_TEST(test_hx711_read);
}