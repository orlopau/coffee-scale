#include <unity.h>

#include "weighing/zero_tracker.h"

static ZeroTracker *tracker;

void setUp(void)
{
    tracker = new ZeroTracker();
    tracker->setWindow(0.1f);
}

void tearDown(void) { delete tracker; }

/** Tracks a steady weight at 10 Hz from 0 ms on, and returns the last correction. */
static float trackSteady(float grams, int readings)
{
    float correction = 0;
    for (int i = 0; i < readings; i++)
    {
        correction = tracker->track(i * 100, grams, 0);
    }
    return correction;
}

void test_first_reading_moves_nothing(void) { TEST_ASSERT_EQUAL_FLOAT(0, tracker->track(0, 0.05f, 0)); }

// 0.01 g/s for 100 ms
void test_moves_toward_a_positive_weight_at_the_speed_limit(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.00001, 0.001, trackSteady(0.05f, 2));
}

void test_moves_toward_a_negative_weight_at_the_speed_limit(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.00001, -0.001, trackSteady(-0.05f, 2));
}

void test_moves_a_tiny_weight_all_the_way_to_zero(void)
{
    TEST_ASSERT_FLOAT_WITHIN(0.000001, 0.0004, trackSteady(0.0004f, 2));
}

void test_tracks_up_to_the_window(void) { TEST_ASSERT_FLOAT_WITHIN(0.00001, 0.001, trackSteady(0.1f, 2)); }

void test_leaves_a_weight_outside_the_window(void)
{
    TEST_ASSERT_EQUAL_FLOAT(0, trackSteady(0.13f, 2));
    TEST_ASSERT_EQUAL_FLOAT(0, trackSteady(-0.13f, 2));
}

void test_leaves_the_weight_while_it_flows(void)
{
    tracker->track(0, 0.05f, 0);
    TEST_ASSERT_EQUAL_FLOAT(0, tracker->track(100, 0.05f, 0.2f));
    TEST_ASSERT_EQUAL_FLOAT(0, tracker->track(200, 0.05f, -0.2f));
}

void test_tracks_under_the_steadiness_limit(void)
{
    tracker->track(0, 0.05f, 0);
    TEST_ASSERT_FLOAT_WITHIN(0.00001, 0.001, tracker->track(100, 0.05f, 0.09f));
}

void test_window_zero_turns_tracking_off(void)
{
    tracker->setWindow(0);
    TEST_ASSERT_EQUAL_FLOAT(0, trackSteady(0.001f, 2));
}

void test_held_moves_nothing(void)
{
    tracker->track(0, 0.05f, 0);
    tracker->hold(true);
    TEST_ASSERT_EQUAL_FLOAT(0, tracker->track(100, 0.05f, 0));
}

void test_moves_again_once_released(void)
{
    tracker->track(0, 0.05f, 0);
    tracker->hold(true);
    tracker->track(100, 0.05f, 0);
    tracker->hold(false);
    TEST_ASSERT_FLOAT_WITHIN(0.00001, 0.001, tracker->track(200, 0.05f, 0));
}

// e.g. the loop was blocked for 5 s: still at most 200 ms worth of tracking
void test_a_long_gap_moves_no_further_than_200_ms(void)
{
    tracker->track(0, 0.05f, 0);
    TEST_ASSERT_FLOAT_WITHIN(0.00001, 0.002, tracker->track(5000, 0.05f, 0));
}

void test_same_time_twice_moves_nothing(void)
{
    tracker->track(100, 0.05f, 0);
    TEST_ASSERT_EQUAL_FLOAT(0, tracker->track(100, 0.05f, 0));
}

void test_reset_starts_over(void)
{
    tracker->track(0, 0.05f, 0);
    tracker->reset();
    TEST_ASSERT_EQUAL_FLOAT(0, tracker->track(100, 0.05f, 0));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_first_reading_moves_nothing);
    RUN_TEST(test_moves_toward_a_positive_weight_at_the_speed_limit);
    RUN_TEST(test_moves_toward_a_negative_weight_at_the_speed_limit);
    RUN_TEST(test_moves_a_tiny_weight_all_the_way_to_zero);
    RUN_TEST(test_tracks_up_to_the_window);
    RUN_TEST(test_leaves_a_weight_outside_the_window);
    RUN_TEST(test_leaves_the_weight_while_it_flows);
    RUN_TEST(test_tracks_under_the_steadiness_limit);
    RUN_TEST(test_window_zero_turns_tracking_off);
    RUN_TEST(test_held_moves_nothing);
    RUN_TEST(test_moves_again_once_released);
    RUN_TEST(test_a_long_gap_moves_no_further_than_200_ms);
    RUN_TEST(test_same_time_twice_moves_nothing);
    RUN_TEST(test_reset_starts_over);
    return UNITY_END();
}
