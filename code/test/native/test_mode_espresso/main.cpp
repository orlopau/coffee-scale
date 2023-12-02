#include "mocks.h"
#include "modes/mode_espresso.h"
#include "util/stopwatch.h"
#include <unity.h>

#include <chrono>
#include <thread>

static MockWeightSensor *weightSensor;
static MockButtons *buttons;
static MockDisplay *display;
static Stopwatch *stopwatch;

ModeEspresso *modeEspresso;

void setUp(void)
{
    weightSensor = new MockWeightSensor();
    buttons = new MockButtons();
    display = new MockDisplay();
    stopwatch = new Stopwatch();
    modeEspresso = new ModeEspresso(*weightSensor, *buttons, *display, *stopwatch);
}

void tearDown(void)
{
    delete buttons;
    delete display;
    delete modeEspresso;
    delete weightSensor;
}

void test_encoder_adjusts_target_weight(void)
{
    modeEspresso->update();
    uint32_t targetWeight = display->espressoTargetWeightMg;

    // turn encoder right increases weighht by 0.1g per tick
    buttons->encoderTicks = 5;
    modeEspresso->update();
    TEST_ASSERT_EQUAL(targetWeight + 500, display->espressoTargetWeightMg);

    // turn encoder left decreases weight by 0.1g per tick
    buttons->encoderTicks = -10;
    modeEspresso->update();
    TEST_ASSERT_EQUAL(targetWeight - 500, display->espressoTargetWeightMg);
}

void test_encoder_clamps_weight_between_min_and_max(void)
{
    modeEspresso->update();
    uint32_t targetWeight = display->espressoTargetWeightMg;

    // turn encoder left
    buttons->encoderTicks = -100000;
    modeEspresso->update();
    TEST_ASSERT_EQUAL(MIN_TARGET_WEIGHT_MG, display->espressoTargetWeightMg);

    // turn encoder right
    buttons->encoderTicks = 1000000;
    modeEspresso->update();
    TEST_ASSERT_EQUAL(MAX_TARGET_WEIGHT_MG, display->espressoTargetWeightMg);
}

void test_clicking_encoder_starts_stopwatch(void)
{
    // initially time on watch is 0
    modeEspresso->update();
    TEST_ASSERT_EQUAL(0, display->espressoCurrentTimeMs);

    // click encoder to start stopwatch
    buttons->encoderClick = ClickType::SINGLE;
    modeEspresso->update();
    TEST_ASSERT_TRUE(stopwatch->isRunning());

    // click again to stop stopwatch
    buttons->encoderClick = ClickType::SINGLE;
    modeEspresso->update();
    TEST_ASSERT_FALSE(stopwatch->isRunning());
}

void test_display_shows_current_weight(void)
{
    weightSensor->weight = 10;
    modeEspresso->update();
    TEST_ASSERT_EQUAL(10 * 1000, display->espressoCurrentWeightMg);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_encoder_adjusts_target_weight);
    RUN_TEST(test_clicking_encoder_starts_stopwatch);
    RUN_TEST(test_encoder_clamps_weight_between_min_and_max);
    RUN_TEST(test_display_shows_current_weight);
    UNITY_END();
}