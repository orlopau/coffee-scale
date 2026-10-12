#include <unity.h>

#include <initializer_list>

#include "detectors/lift_detector.h"

static LiftDetector *detector;
static uint32_t ms;
static float weightBefore;

void setUp(void)
{
    detector = new LiftDetector();
    detector->setDepth(0.7f);
    ms = 1000;
    weightBefore = -1;
}

void tearDown(void) { delete detector; }

/** Feeds readings 90 ms apart, like the HX711, and returns how many lifts were reported. */
static int feed(std::initializer_list<float> readings)
{
    int lifts = 0;
    for (float grams : readings)
    {
        ms += 90;
        if (detector->update(ms, grams, weightBefore))
        {
            lifts++;
        }
    }
    return lifts;
}

/** Feeds the same weight count times. */
static int steady(float grams, int count)
{
    int lifts = 0;
    for (int i = 0; i < count; i++)
    {
        lifts += feed({grams});
    }
    return lifts;
}

// lift 1 of the empty-scale recording: -1.03, -2.27, then -0.29
void test_a_lift_on_the_empty_scale(void)
{
    TEST_ASSERT_EQUAL(0, steady(0, 10));
    TEST_ASSERT_EQUAL(1, feed({-1.03f, -2.27f, -0.29f}));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0, weightBefore);
}

// the first lift with the cup: 152.47, 142.54, 151.85, then 155.83
void test_a_lift_with_a_cup(void)
{
    TEST_ASSERT_EQUAL(0, steady(155.85f, 10));
    TEST_ASSERT_EQUAL(1, feed({152.47f, 142.54f, 151.85f, 155.83f}));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 155.85, weightBefore);
}

void test_weight_before_is_the_mean_of_the_steady_stretch(void)
{
    for (int i = 0; i < 5; i++)
    {
        feed({0.0f, 0.2f});
    }
    TEST_ASSERT_EQUAL(1, feed({-1.5f, 0.1f}));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.1, weightBefore);
}

void test_weight_before_averages_the_last_32_readings(void)
{
    steady(0.0f, 8);
    steady(0.2f, 32);
    TEST_ASSERT_EQUAL(1, feed({-1.5f, 0.2f}));
    // all 40 would give 0.16
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.2, weightBefore);
}

// From the recording with the cup: pressing the scale's button pushes it
// down by 0.7 g, and it springs back 0.37 g below the steady weight.
void test_the_buttons_spring_back_is_no_lift(void)
{
    detector->setDepth(0.5f); // Gentle
    steady(155.70f, 10);
    TEST_ASSERT_EQUAL(0, feed({156.53f, 156.13f, 155.36f, 155.80f, 155.84f, 155.84f}));
    steady(0, 10);
    TEST_ASSERT_EQUAL(0, feed({-0.37f, 0.0f}));
}

void test_five_readings_down_is_a_lift_six_is_not(void)
{
    steady(0, 10);
    TEST_ASSERT_EQUAL(1, feed({-1, -1, -1, -1, -1, 0}));
    steady(0, 10);
    TEST_ASSERT_EQUAL(0, feed({-1, -1, -1, -1, -1, -1, 0}));
}

// the cup taken off and put straight back took 10 readings
void test_a_cup_taken_off_is_no_lift_but_the_next_lift_is(void)
{
    steady(155.85f, 10);
    TEST_ASSERT_EQUAL(0, feed({115.53f, 25.24f, 0.35f, 0.09f, 0.07f, 0.03f, 0.03f}));
    TEST_ASSERT_EQUAL(0, steady(0.03f, 10));
    TEST_ASSERT_EQUAL(1, feed({-1.5f, 0.03f}));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.03, weightBefore);
}

void test_no_lift_without_a_steady_stretch_before(void)
{
    // right after the start
    TEST_ASSERT_EQUAL(0, feed({0, -1.5f, 0}));
    // right after something moved
    steady(0, 10);
    TEST_ASSERT_EQUAL(0, feed({0.5f, 0, -1.5f, 0}));
}

// a double tap of the first recording, it never goes below rest
void test_a_tap_is_no_lift(void)
{
    steady(0, 10);
    TEST_ASSERT_EQUAL(0, feed({2.01f, 12.98f, 11.74f, 22.14f, 5.95f, -0.04f, -0.03f, 0.08f, 0.08f}));
}

void test_firm_needs_a_deeper_lift(void)
{
    detector->setDepth(1.5f);
    steady(0, 10);
    TEST_ASSERT_EQUAL(0, feed({-0.96f, -0.17f}));
    steady(0, 10);
    TEST_ASSERT_EQUAL(1, feed({-1.6f, 0}));
}

void test_off_never_reports(void)
{
    detector->setDepth(0);
    steady(0, 10);
    TEST_ASSERT_EQUAL(0, feed({-3.9f, 0}));
}

void test_reset_forgets_a_dip(void)
{
    steady(0, 10);
    feed({-1.5f});
    detector->reset();
    TEST_ASSERT_EQUAL(0, feed({0}));
}

void test_a_gap_resets(void)
{
    steady(0, 10);
    feed({-1.5f});
    ms += 600;
    TEST_ASSERT_EQUAL(0, feed({0}));
}

void test_one_report_per_lift(void)
{
    steady(0, 10);
    TEST_ASSERT_EQUAL(1, feed({-1.5f, 0}));
    TEST_ASSERT_EQUAL(0, steady(0, 20));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_a_lift_on_the_empty_scale);
    RUN_TEST(test_a_lift_with_a_cup);
    RUN_TEST(test_weight_before_is_the_mean_of_the_steady_stretch);
    RUN_TEST(test_weight_before_averages_the_last_32_readings);
    RUN_TEST(test_the_buttons_spring_back_is_no_lift);
    RUN_TEST(test_five_readings_down_is_a_lift_six_is_not);
    RUN_TEST(test_a_cup_taken_off_is_no_lift_but_the_next_lift_is);
    RUN_TEST(test_no_lift_without_a_steady_stretch_before);
    RUN_TEST(test_a_tap_is_no_lift);
    RUN_TEST(test_firm_needs_a_deeper_lift);
    RUN_TEST(test_off_never_reports);
    RUN_TEST(test_reset_forgets_a_dip);
    RUN_TEST(test_a_gap_resets);
    RUN_TEST(test_one_report_per_lift);
    return UNITY_END();
}
