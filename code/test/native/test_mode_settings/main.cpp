#include <unity.h>

#include "mock/mock_display.h"
#include "mock/mock_interface.h"
#include "mocks.h"
#include "modes/mode_settings.h"
#include "settings.h"

static ModeSettings *modeSettings;

void setUp(void)
{
    Interface::reset();
    Display::reset();
    modeSettings = new ModeSettings();
    modeSettings->enter();
}

void tearDown(void) { delete modeSettings; }

/** Opens a setting and turns its value by ticks, from 0 (the mock stores nothing). */
static void edit(Settings::FloatSettingID id, int ticks)
{
    Interface::encoderTurns = id;
    modeSettings->update();
    Interface::reset();
    Interface::encoderClick = ClickType::SINGLE;
    modeSettings->update();
    Interface::reset();
    Interface::encoderTicks = ticks;
    modeSettings->update();
    modeSettings->render(Display::mockCanvas);
}

void test_value_shows_one_decimal_for_steps_of_a_tenth(void)
{
    edit(Settings::AVERAGING_DELTA, 3);
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("0.3"));
}

void test_value_shows_two_decimals_for_steps_of_a_twentieth(void)
{
    edit(Settings::ZERO_TRACKING, 7);
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("0.35"));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_value_shows_one_decimal_for_steps_of_a_tenth);
    RUN_TEST(test_value_shows_two_decimals_for_steps_of_a_twentieth);
    return UNITY_END();
}
