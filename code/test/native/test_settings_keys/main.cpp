#include <string.h>

#include <set>
#include <string>

#include "unity.h"
#include "settings.h"

void setUp(void) {}

void tearDown(void) {}

static void assertKeyFitsNvs(const char *key)
{
    TEST_ASSERT_NOT_NULL(key);
    TEST_ASSERT_GREATER_THAN(0, strlen(key));
    TEST_ASSERT_LESS_OR_EQUAL_MESSAGE(15, strlen(key), key);
}

// NVS keys are at most 15 characters. A longer one only fails on the device, at runtime.
void test_keys_fit_nvs(void)
{
    for (const auto &setting : Settings::floatSettings)
    {
        assertKeyFitsNvs(setting.key);
    }
    for (const auto &setting : Settings::choiceSettings)
    {
        assertKeyFitsNvs(setting.key);
    }
    assertKeyFitsNvs(Settings::CALIBRATION_KEY);
    assertKeyFitsNvs(Settings::WIFI_ASKED_KEY);
    assertKeyFitsNvs(Settings::ROLLBACK_SHOWN_KEY);
}

// one namespace for both kinds
void test_keys_unique(void)
{
    std::set<std::string> keys = {Settings::CALIBRATION_KEY, Settings::WIFI_ASKED_KEY, Settings::ROLLBACK_SHOWN_KEY};
    TEST_ASSERT_EQUAL(3, keys.size());
    for (const auto &setting : Settings::floatSettings)
    {
        TEST_ASSERT_TRUE_MESSAGE(keys.insert(setting.key).second, setting.key);
    }
    for (const auto &setting : Settings::choiceSettings)
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
    for (size_t i = 0; i < Settings::choiceSettings.size(); i++)
    {
        TEST_ASSERT_EQUAL(i, Settings::choiceSettings[i].id);
    }
}

void test_choices_have_labels_and_a_valid_default(void)
{
    for (const auto &setting : Settings::choiceSettings)
    {
        TEST_ASSERT_GREATER_THAN(1, setting.count);
        TEST_ASSERT_LESS_THAN(setting.count, setting.def);
        for (int i = 0; i < setting.count; i++)
        {
            TEST_ASSERT_NOT_NULL(setting.labels[i]);
        }
    }
    const auto &lift = Settings::choiceSettings[Settings::LIFT_TARE];
    TEST_ASSERT_EQUAL_STRING(SETTINGS_LIFT_TARE_NORMAL, lift.labels[lift.def]);
    TEST_ASSERT_EQUAL_FLOAT(0.7f, Settings::LIFT_TARE_DEPTHS[lift.def]);
}

// e.g. stored by a later firmware with more levels
void test_choice_out_of_range_is_the_default(void)
{
    const auto &lift = Settings::choiceSettings[Settings::LIFT_TARE];
    TEST_ASSERT_EQUAL(lift.def, Settings::validChoice(lift, 7));
    TEST_ASSERT_EQUAL(lift.def, Settings::validChoice(lift, -1));
    TEST_ASSERT_EQUAL(3, Settings::validChoice(lift, 3));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_keys_fit_nvs);
    RUN_TEST(test_keys_unique);
    RUN_TEST(test_ids_match_positions);
    RUN_TEST(test_choices_have_labels_and_a_valid_default);
    RUN_TEST(test_choice_out_of_range_is_the_default);
    UNITY_END();
}
