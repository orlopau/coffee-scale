#include "unity.h"
#include "stopwatch.h"

#include "millis.h"

void setUp(void) {}

void tearDown(void) {}

void test_stopwatch_start(void)
{
    Stopwatch stopwatch;
    stopwatch.start();
    TEST_ASSERT_TRUE(stopwatch.isRunning());
    sleep_for(2);
    TEST_ASSERT_GREATER_OR_EQUAL(2, stopwatch.getTime());
}

void test_stopwatch_stop(void)
{
    Stopwatch stopwatch;
    stopwatch.start();
    sleep_for(2);
    stopwatch.stop();
    TEST_ASSERT_FALSE(stopwatch.isRunning());
    TEST_ASSERT_GREATER_OR_EQUAL(2, stopwatch.getTime());
}

void test_stopwatch_toggle(void)
{
    Stopwatch stopwatch;
    stopwatch.toggle();
    TEST_ASSERT_TRUE(stopwatch.isRunning());
    sleep_for(2);
    TEST_ASSERT_GREATER_OR_EQUAL(2, stopwatch.getTime());
    stopwatch.toggle();
    TEST_ASSERT_FALSE(stopwatch.isRunning());
    sleep_for(2);
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

void test_stopwatch_start_at_counts_from_the_past(void)
{
    set_now(10000);
    Stopwatch stopwatch;
    stopwatch.startAt(7600);
    TEST_ASSERT_TRUE(stopwatch.isRunning());
    TEST_ASSERT_EQUAL(2400, stopwatch.getTime());
}

// e.g. leaving the espresso mode after the shot reached its target
void test_stopwatch_stop_again_keeps_the_time(void)
{
    set_now(10000);
    Stopwatch stopwatch;
    stopwatch.start();
    set_now(13000);
    stopwatch.stop();
    set_now(20000);
    stopwatch.stop();
    TEST_ASSERT_EQUAL(3000, stopwatch.getTime());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_stopwatch_start);
    RUN_TEST(test_stopwatch_stop);
    RUN_TEST(test_stopwatch_toggle);
    RUN_TEST(test_stopwatch_reset);
    RUN_TEST(test_stopwatch_getTime);
    RUN_TEST(test_stopwatch_start_at_counts_from_the_past);
    RUN_TEST(test_stopwatch_stop_again_keeps_the_time);
    UNITY_END();
}