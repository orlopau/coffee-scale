#include "unity.h"
#include "mode.h"
#include "mocks.h"
#include "stopwatch.h"

#include <chrono>
#include <thread>

Stopwatch *stopwatch;
MockLoadCell *loadCell;
MockButtons *buttons;
MockDisplay *display;
ModeDefault *modeDefault;

void setUp(void)
{
    stopwatch = new Stopwatch();
    loadCell = new MockLoadCell();
    buttons = new MockButtons();
    display = new MockDisplay();
    modeDefault = new ModeDefault(*loadCell, *buttons, *display, *stopwatch);
}

void tearDown(void)
{
    delete stopwatch;
    delete loadCell;
    delete buttons;
    delete display;
    delete modeDefault;
}

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

void run_all_mode_default()
{
    RUN_TEST(test_stopwatch_start_when_click);
    RUN_TEST(test_stopwatch_stop_when_click_again);

    RUN_TEST(test_loadcell_tare_when_click);
    RUN_TEST(test_display_shows_weight);
    RUN_TEST(test_display_shows_time);
}