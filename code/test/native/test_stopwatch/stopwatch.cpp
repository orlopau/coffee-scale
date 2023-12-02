#include "unity.h"
#include "util/stopwatch.h"

#include <chrono>
#include <thread>

void setUp(void) {}

void tearDown(void) {}

void test_stopwatch_start(void)
{
    Stopwatch stopwatch;
    stopwatch.start();
    TEST_ASSERT_TRUE(stopwatch.isRunning());
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    TEST_ASSERT_GREATER_OR_EQUAL(2, stopwatch.getTime());
}

void test_stopwatch_stop(void)
{
    Stopwatch stopwatch;
    stopwatch.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    stopwatch.stop();
    TEST_ASSERT_FALSE(stopwatch.isRunning());
    TEST_ASSERT_GREATER_OR_EQUAL(2, stopwatch.getTime());
}

void test_stopwatch_toggle(void)
{
    Stopwatch stopwatch;
    stopwatch.toggle();
    TEST_ASSERT_TRUE(stopwatch.isRunning());
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    TEST_ASSERT_GREATER_OR_EQUAL(2, stopwatch.getTime());
    stopwatch.toggle();
    TEST_ASSERT_FALSE(stopwatch.isRunning());
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    TEST_ASSERT_GREATER_OR_EQUAL(2, stopwatch.getTime());
}

void test_stopwatch_reset(void)
{
    Stopwatch stopwatch;
    stopwatch.start();
    stopwatch.reset();
    TEST_ASSERT_FALSE(stopwatch.isRunning());
    TEST_ASSERT_EQUAL(0, stopwatch.getTime());
}

void test_stopwatch_getTime(void)
{
    Stopwatch stopwatch;
    TEST_ASSERT_EQUAL(0, stopwatch.getTime());
    stopwatch.start();
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_stopwatch_start);
    RUN_TEST(test_stopwatch_stop);
    RUN_TEST(test_stopwatch_toggle);
    RUN_TEST(test_stopwatch_reset);
    RUN_TEST(test_stopwatch_getTime);
    UNITY_END();
}