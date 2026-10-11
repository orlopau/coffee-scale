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

/** Averages 10 g at 0, 100 and 200 ms, with a threshold of 1 g/s. */
static void averageTen(AdaptiveAverage &average)
{
    average.configure(1, 4);
    average.add(0, 10);
    average.add(100, 10);
    average.add(200, 10);
}

// The weight is still, only one reading jumps: noise, not a change.
void test_a_single_fast_reading_while_averaging_is_ignored(void)
{
    AdaptiveAverage average;
    averageTen(average);
    TEST_ASSERT_EQUAL_FLOAT(10, average.add(300, 10.5f));
    TEST_ASSERT_EQUAL_FLOAT(10, average.add(400, 10));
}

void test_an_ignored_reading_stays_out_of_the_average(void)
{
    AdaptiveAverage average;
    averageTen(average);
    average.add(300, 10.5f);
    // 0.05 g in one sample's time is slow: averaged over 10, 10, 10.05
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 10.0167, average.add(400, 10.05f));
}

void test_two_fast_readings_in_a_row_end_averaging(void)
{
    AdaptiveAverage average;
    averageTen(average);
    TEST_ASSERT_EQUAL_FLOAT(10, average.add(300, 30));
    TEST_ASSERT_EQUAL_FLOAT(30, average.add(400, 30));
}

// A small weight placed on the scale, e.g. a bean: fast for one sample's time,
// but over the two samples since the last still one it looks slow.
void test_a_small_step_confirmed_by_the_next_reading_ends_averaging(void)
{
    AdaptiveAverage average;
    averageTen(average);
    TEST_ASSERT_EQUAL_FLOAT(10, average.add(300, 10.15f));
    TEST_ASSERT_EQUAL_FLOAT(10.15f, average.add(400, 10.15f));
}

// The last sample before was the noisy one: the two after it are both far from
// it, but not from the weight averaged so far.
void test_two_normal_readings_after_a_noisy_one_keep_averaging(void)
{
    AdaptiveAverage average;
    average.configure(1, 4);
    average.add(0, 10);
    average.add(100, 10);
    average.add(200, 10);
    average.add(300, 9.95f);
    float before = average.value();
    TEST_ASSERT_EQUAL_FLOAT(before, average.add(400, 10.06f));
    // 0.06 g in 100 ms from 9.95 g, but only ~0.05 g from the average
    TEST_ASSERT_TRUE(average.add(500, 10.06f) < 10.04f);
}

void test_a_fast_reading_shows_at_once_when_not_averaging(void)
{
    AdaptiveAverage average;
    average.configure(1, 4);
    average.add(0, 10);
    TEST_ASSERT_EQUAL_FLOAT(30, average.add(100, 30));
}

void test_same_time_twice_counts_as_fast_change(void)
{
    AdaptiveAverage average;
    average.configure(1, 4);
    average.add(0, 10);
    average.add(100, 10);
    float value = average.add(100, 10.5f);
    TEST_ASSERT_FALSE(std::isnan(value));
    TEST_ASSERT_EQUAL_FLOAT(10, value);
    TEST_ASSERT_EQUAL_FLOAT(10.5f, average.add(200, 10.5f));
}

void test_time_running_backwards_counts_as_fast_change(void)
{
    AdaptiveAverage average;
    average.configure(1, 4);
    average.add(1000, 10);
    average.add(1100, 10);
    TEST_ASSERT_EQUAL_FLOAT(10, average.add(900, 10.2f));
    TEST_ASSERT_EQUAL_FLOAT(10.2f, average.add(1200, 10.2f));
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
    RUN_TEST(test_a_single_fast_reading_while_averaging_is_ignored);
    RUN_TEST(test_an_ignored_reading_stays_out_of_the_average);
    RUN_TEST(test_two_fast_readings_in_a_row_end_averaging);
    RUN_TEST(test_a_small_step_confirmed_by_the_next_reading_ends_averaging);
    RUN_TEST(test_two_normal_readings_after_a_noisy_one_keep_averaging);
    RUN_TEST(test_a_fast_reading_shows_at_once_when_not_averaging);
    RUN_TEST(test_same_time_twice_counts_as_fast_change);
    RUN_TEST(test_time_running_backwards_counts_as_fast_change);
    RUN_TEST(test_configure_again_starts_over);
    return UNITY_END();
}
