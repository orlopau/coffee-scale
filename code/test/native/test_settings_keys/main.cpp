#include <string.h>

#include <set>
#include <string>

#include "unity.h"
#include "settings.h"

void setUp(void) {}

void tearDown(void) {}

// NVS keys are at most 15 characters. A longer one only fails on the device, at runtime.
void test_keys_fit_nvs(void)
{
    for (const auto &setting : Settings::floatSettings)
    {
        TEST_ASSERT_NOT_NULL(setting.key);
        TEST_ASSERT_GREATER_THAN(0, strlen(setting.key));
        TEST_ASSERT_LESS_OR_EQUAL_MESSAGE(15, strlen(setting.key), setting.key);
    }
    TEST_ASSERT_LESS_OR_EQUAL(15, strlen(Settings::CALIBRATION_KEY));
}

void test_keys_unique(void)
{
    std::set<std::string> keys = {Settings::CALIBRATION_KEY};
    for (const auto &setting : Settings::floatSettings)
    {
        TEST_ASSERT_TRUE_MESSAGE(keys.insert(setting.key).second, setting.key);
    }
}

// The settings menu and the fakes index by id, and the EEPROM migration reads the old offsets by id.
void test_ids_match_positions(void)
{
    for (size_t i = 0; i < Settings::floatSettings.size(); i++)
    {
        TEST_ASSERT_EQUAL(i, Settings::floatSettings[i].id);
    }
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_keys_fit_nvs);
    RUN_TEST(test_keys_unique);
    RUN_TEST(test_ids_match_positions);
    UNITY_END();
}
