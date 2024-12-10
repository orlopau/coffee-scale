#include "mock/mock_interface.h"
#include "mock/mock_display.h"
#include "mocks.h"
#include "modes/mode_espresso.h"
#include "stopwatch.h"
#include <unity.h>

#include <chrono>
#include <thread>

static MockWeightSensor *weightSensor;
static Stopwatch *stopwatch;

ModeEspresso *modeEspresso;

void setUp(void)
{
    Display::reset();
    Interface::reset();
    weightSensor = new MockWeightSensor();
    stopwatch = new Stopwatch();
    modeEspresso = new ModeEspresso(*weightSensor, *stopwatch);
}

void tearDown(void)
{
    delete modeEspresso;
    delete weightSensor;
}

void test_encoder_adjusts_target_weight(void)
{
    modeEspresso->update();
    uint32_t targetWeight = Display::espressoTargetWeightMg;

    // turn encoder right increases weighht by 0.1g per tick
    Interface::encoderTicks = 5;
    modeEspresso->update();
    TEST_ASSERT_EQUAL(targetWeight + 500, Display::espressoTargetWeightMg);

    // turn encoder left decreases weight by 0.1g per tick
    Interface::encoderTicks = -10;
    modeEspresso->update();
    TEST_ASSERT_EQUAL(targetWeight - 500, Display::espressoTargetWeightMg);
}

void test_encoder_clamps_weight_between_min_and_max(void)
{
    modeEspresso->update();
    uint32_t targetWeight = Display::espressoTargetWeightMg;

    // turn encoder left
    Interface::encoderTicks = -100000;
    modeEspresso->update();
    TEST_ASSERT_EQUAL(MIN_TARGET_WEIGHT_MG, Display::espressoTargetWeightMg);

    // turn encoder right
    Interface::encoderTicks = 1000000;
    modeEspresso->update();
    TEST_ASSERT_EQUAL(MAX_TARGET_WEIGHT_MG, Display::espressoTargetWeightMg);
}

void test_clicking_encoder_starts_stopwatch(void)
{
    // initially time on watch is 0
    modeEspresso->update();
    TEST_ASSERT_EQUAL(0, Display::espressoCurrentTimeMs);

    // click encoder to start stopwatch
    Interface::encoderClick = ClickType::SINGLE;
    modeEspresso->update();
    TEST_ASSERT_TRUE(stopwatch->isRunning());

    // click again to stop stopwatch
    Interface::encoderClick = ClickType::SINGLE;
    modeEspresso->update();
    TEST_ASSERT_FALSE(stopwatch->isRunning());
}

void test_display_shows_current_weight(void)
{
    weightSensor->weight = 10;
    modeEspresso->update();
    TEST_ASSERT_EQUAL(10 * 1000, Display::espressoCurrentWeightMg);
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