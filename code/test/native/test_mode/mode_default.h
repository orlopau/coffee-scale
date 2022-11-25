#include "unity.h"
#include "mode.h"
#include "mocks.h"
#include "stopwatch.h"
#include "globals.h"

#include <chrono>
#include <thread>

ModeDefault *modeDefault;

void test_stopwatch_start_when_click(void)
{
    TEST_ASSERT_FALSE(stopwatch->isRunning());
    buttons->encoderClick = ClickType::SINGLE;
    modeDefault->update();
    TEST_ASSERT_TRUE(stopwatch->isRunning());
}

void test_stopwatch_stop_when_click_again(void)
{
    buttons->encoderClick = ClickType::SINGLE;
    modeDefault->update();
    modeDefault->update();
    TEST_ASSERT_FALSE(stopwatch->isRunning());
}

void test_loadcell_tare_when_click(void)
{
    loadCell->weight = 1.0;

    buttons->bootClick = ClickType::SINGLE;
    modeDefault->update();
    TEST_ASSERT_EQUAL(0, loadCell->weight);
}

void test_display_shows_weight(void)
{
    loadCell->weight = 1.0;
    modeDefault->update();
    TEST_ASSERT_EQUAL(1.0, display->weight);
}

void test_display_shows_time(void)
{
    stopwatch->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    modeDefault->update();
    TEST_ASSERT_GREATER_OR_EQUAL(2, display->time);
}

void setUpDefault(void)
{
    modeDefault = new ModeDefault(*loadCell, *buttons, *display, *stopwatch);
}

void tearDownDefault(void)
{
    delete modeDefault;
}

void run_all_mode_default()
{
    currSetUp = setUpDefault;
    currTearDown = tearDownDefault;
    RUN_TEST(test_stopwatch_start_when_click);
    RUN_TEST(test_stopwatch_stop_when_click_again);

    RUN_TEST(test_loadcell_tare_when_click);
    RUN_TEST(test_display_shows_weight);
    RUN_TEST(test_display_shows_time);
}