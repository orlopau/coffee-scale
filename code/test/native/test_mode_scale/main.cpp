#include <unity.h>
#include "modes/mode_scale.h"
#include "mocks.h"
#include "stopwatch.h"

#include <chrono>
#include <thread>

static Stopwatch *stopwatch;
static MockWeightSensor *weightSensor;
static MockButtons *buttons;
static MockDisplay *display;

ModeScale *modeScale;

void setUp(void)
{
    stopwatch = new Stopwatch();
    weightSensor = new MockWeightSensor();
    buttons = new MockButtons();
    display = new MockDisplay();
    modeScale = new ModeScale(*weightSensor, *buttons, *display, *stopwatch);
}

void tearDown(void)
{
    delete stopwatch;
    delete buttons;
    delete display;
    delete modeScale;
    delete weightSensor;
}

void test_stopwatch_start_when_click(void)
{
    TEST_ASSERT_FALSE(stopwatch->isRunning());
    buttons->encoderClick = ClickType::SINGLE;
    modeScale->update();
    TEST_ASSERT_TRUE(stopwatch->isRunning());
}

void test_stopwatch_stop_when_click_again(void)
{
    buttons->encoderClick = ClickType::SINGLE;
    modeScale->update();
    modeScale->update();
    TEST_ASSERT_FALSE(stopwatch->isRunning());
}

void test_loadcell_tare_when_encoder_rotated(void)
{
    weightSensor->weight = 1;

    buttons->encoderDirection = EncoderDirection::CW;
    modeScale->update();
    TEST_ASSERT_EQUAL(0, weightSensor->getWeight());
}

void test_display_shows_weight(void)
{
    weightSensor->weight = 1;

    modeScale->update();
    TEST_ASSERT_EQUAL(1.0, display->weight);
}

void test_display_shows_time(void)
{
    stopwatch->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    modeScale->update();
    TEST_ASSERT_GREATER_OR_EQUAL(2, display->time);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_stopwatch_start_when_click);
    RUN_TEST(test_stopwatch_stop_when_click_again);

    RUN_TEST(test_loadcell_tare_when_encoder_rotated);
    RUN_TEST(test_display_shows_weight);
    RUN_TEST(test_display_shows_time);
    UNITY_END();
}