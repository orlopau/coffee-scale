#include <unity.h>

#include "mock/mock_display.h"
#include "mock/mock_interface.h"
#include "mocks.h"
#include "modes/mode_settings.h"
#include "settings.h"

static ModeSettings *modeSettings;

static int actionRuns = 0;
static void runAction() { actionRuns++; }
static int leaves = 0;
static void onLeave() { leaves++; }
static const SettingsAction ACTIONS[] = {{"Wi-Fi", runAction}};

static const Settings::ChoiceSetting &liftTare() { return Settings::choiceSettings[Settings::LIFT_TARE]; }

void setUp(void)
{
    Interface::reset();
    Display::reset();
    modeSettings = new ModeSettings(ACTIONS, 1);
    actionRuns = 0;
    modeSettings->enter();
}

void tearDown(void)
{
    delete modeSettings;
    Settings::setChoice(liftTare(), liftTare().def);
}

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

/** Opens a choice setting, they come after the float settings, and turns it by ticks. */
static void editChoice(Settings::ChoiceSettingID id, int ticks)
{
    Interface::encoderTurns = Settings::FLOAT_SETTING_NUM + id;
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

void test_choice_shows_its_label(void)
{
    editChoice(Settings::LIFT_TARE, 0);
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText(SETTINGS_LIFT_TARE_NORMAL));
}

void test_choice_stops_at_the_last_label(void)
{
    editChoice(Settings::LIFT_TARE, 5);
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText(SETTINGS_LIFT_TARE_FIRM));

    // one detent back leaves the last label right away
    Interface::encoderTicks -= 1;
    modeSettings->update();
    Display::reset();
    modeSettings->render(Display::mockCanvas);
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText(SETTINGS_LIFT_TARE_NORMAL));
}

void test_choice_stops_at_the_first_label(void)
{
    editChoice(Settings::LIFT_TARE, -5);
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText(SETTINGS_LIFT_TARE_OFF));
}

void test_click_stores_the_choice(void)
{
    editChoice(Settings::LIFT_TARE, -1);
    Interface::encoderClick = ClickType::SINGLE;
    modeSettings->update();
    TEST_ASSERT_EQUAL(1, Settings::getChoice(liftTare()));
}

void test_long_press_keeps_the_stored_choice(void)
{
    editChoice(Settings::LIFT_TARE, 1);
    Interface::encoderClick = ClickType::LONG;
    modeSettings->update();
    TEST_ASSERT_EQUAL(liftTare().def, Settings::getChoice(liftTare()));
}

void test_actions_come_after_the_settings(void)
{
    Interface::encoderTurns = 100;
    modeSettings->update();
    Display::reset();
    modeSettings->render(Display::mockCanvas);
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("Wi-Fi"));
}

void test_click_runs_the_action(void)
{
    Interface::encoderTurns = Settings::SETTING_COUNT;
    modeSettings->update();
    Interface::reset();
    Interface::encoderClick = ClickType::SINGLE;
    modeSettings->update();
    TEST_ASSERT_EQUAL(1, actionRuns);
    // back in the menu, not editing a setting
    TEST_ASSERT_TRUE(modeSettings->canSwitchMode());
}

void test_without_actions_the_menu_ends_with_the_settings(void)
{
    ModeSettings plain;
    plain.enter();
    Interface::encoderTurns = 100;
    plain.update();
    Interface::reset();
    Interface::encoderClick = ClickType::SINGLE;
    plain.update();
    // the last entry is lift-to-tare, a choice being edited now
    TEST_ASSERT_FALSE(plain.canSwitchMode());
    TEST_ASSERT_EQUAL(0, actionRuns);
}

// the changed settings apply without a restart
void test_leaving_tells_that_the_settings_may_have_changed(void)
{
    ModeSettings settings(ACTIONS, 1, onLeave);
    leaves = 0;
    settings.leave();
    TEST_ASSERT_EQUAL(1, leaves);
}

void test_leaving_without_a_callback_does_nothing(void)
{
    ModeSettings settings(ACTIONS, 1);
    settings.leave();
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_value_shows_one_decimal_for_steps_of_a_tenth);
    RUN_TEST(test_value_shows_two_decimals_for_steps_of_a_twentieth);
    RUN_TEST(test_choice_shows_its_label);
    RUN_TEST(test_choice_stops_at_the_last_label);
    RUN_TEST(test_choice_stops_at_the_first_label);
    RUN_TEST(test_click_stores_the_choice);
    RUN_TEST(test_long_press_keeps_the_stored_choice);
    RUN_TEST(test_actions_come_after_the_settings);
    RUN_TEST(test_click_runs_the_action);
    RUN_TEST(test_without_actions_the_menu_ends_with_the_settings);
    RUN_TEST(test_leaving_tells_that_the_settings_may_have_changed);
    RUN_TEST(test_leaving_without_a_callback_does_nothing);
    return UNITY_END();
}
