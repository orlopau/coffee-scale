#include <unity.h>

#include <cmath>

#include "weighing/adaptive_average.h"

void setUp(void) {}

void tearDown(void) {}

void test_without_averaging_gives_the_last_value(void)
{
    AdaptiveAverage average;
    average.add(0, 10);
    TEST_ASSERT_EQUAL_FLOAT(12, average.add(100, 12));
    TEST_ASSERT_EQUAL_FLOAT(12, average.value());
}

void test_averages_while_the_weight_changes_slowly(void)
{
    AdaptiveAverage average;
    // when the weight changes by less than 1000 g/s, average over up to 4 samples
    average.configure(1000, 4);

    // faster than 1000 g/s: the last value
    average.add(0, 100);
    TEST_ASSERT_EQUAL_FLOAT(300, average.add(100, 300));

    // slower: averaging starts, but only over values since it started, so the
    // weight doesn't jump back towards the values before
    average.add(100, 400);
    TEST_ASSERT_EQUAL_FLOAT(450, average.add(200, 450));
    TEST_ASSERT_EQUAL_FLOAT(475, average.add(300, 500));
    TEST_ASSERT_EQUAL_FLOAT(500, average.add(400, 550));
    TEST_ASSERT_EQUAL_FLOAT(525, average.add(500, 600));
    // never more than 4 samples
    TEST_ASSERT_EQUAL_FLOAT(575, average.add(600, 650));
}

void test_a_fast_change_ends_averaging(void)
{
    AdaptiveAverage average;
    average.configure(1, 4);
    average.add(0, 10);
    average.add(100, 10);
    average.add(200, 10);
    TEST_ASSERT_EQUAL_FLOAT(30, average.add(300, 30));
}

void test_same_time_twice_counts_as_fast_change(void)
{
    AdaptiveAverage average;
    average.configure(1, 4);
    average.add(0, 10);
    average.add(100, 10);
    float value = average.add(100, 10.5f);
    TEST_ASSERT_FALSE(std::isnan(value));
    TEST_ASSERT_EQUAL_FLOAT(10.5f, value);
}

void test_time_running_backwards_counts_as_fast_change(void)
{
    AdaptiveAverage average;
    average.configure(1, 4);
    average.add(1000, 10);
    average.add(1100, 10);
    TEST_ASSERT_EQUAL_FLOAT(10.2f, average.add(900, 10.2f));
}

void test_configure_again_starts_over(void)
{
    AdaptiveAverage average;
    average.configure(1000, 4);
    average.add(0, 10);
    average.add(100, 20);
    average.configure(1000, 2);
    average.add(200, 30);
    average.add(300, 40);
    TEST_ASSERT_EQUAL_FLOAT(45, average.add(400, 50));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_without_averaging_gives_the_last_value);
    RUN_TEST(test_averages_while_the_weight_changes_slowly);
    RUN_TEST(test_a_fast_change_ends_averaging);
    RUN_TEST(test_same_time_twice_counts_as_fast_change);
    RUN_TEST(test_time_running_backwards_counts_as_fast_change);
    RUN_TEST(test_configure_again_starts_over);
    return UNITY_END();
}
