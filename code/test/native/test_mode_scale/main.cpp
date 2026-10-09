#include <unity.h>
#include "modes/mode_scale.h"
#include "mocks.h"
#include "mock/mock_interface.h"
#include "mock/mock_display.h"
#include "stopwatch.h"

#include <chrono>
#include <thread>

static Stopwatch *stopwatch;
static MockWeightSensor *weightSensor;

ModeScale *modeScale;

void setUp(void)
{
    Interface::reset();
    Display::reset();
    stopwatch = new Stopwatch();
    weightSensor = new MockWeightSensor();
    modeScale = new ModeScale(*weightSensor, *stopwatch);
}

void tearDown(void)
{
    delete stopwatch;
    delete modeScale;
    delete weightSensor;
}

void test_stopwatch_start_when_click(void)
{
    TEST_ASSERT_FALSE(stopwatch->isRunning());
    Interface::encoderClick = ClickType::SINGLE;
    modeScale->update();
    TEST_ASSERT_TRUE(stopwatch->isRunning());
}

void test_stopwatch_stop_when_click_again(void)
{
    Interface::encoderClick = ClickType::SINGLE;
    modeScale->update();
    modeScale->update();
    TEST_ASSERT_FALSE(stopwatch->isRunning());
}

void test_loadcell_tare_when_encoder_rotated(void)
{
    weightSensor->weight = 1;

    Interface::encoderDirection = Interface::EncoderDirection::CW;
    modeScale->update();
    TEST_ASSERT_EQUAL(0, weightSensor->getWeight());
}

void test_display_shows_weight(void)
{
    weightSensor->weight = 1;

    modeScale->update();
    modeScale->render(Display::mockCanvas);
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("1.00g"));
}

void test_display_shows_time(void)
{
    stopwatch->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    modeScale->update();
    modeScale->render(Display::mockCanvas);
    // at least 0.2 s, allowing the sleep to overshoot
    const char *time = Display::mockCanvas.texts.back().text.c_str();
    TEST_ASSERT_EQUAL_STRING_LEN("00:00.", time, 6);
    TEST_ASSERT_GREATER_OR_EQUAL('2', time[6]);
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