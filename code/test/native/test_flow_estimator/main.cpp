#include <unity.h>

#include <cmath>
#include <random>

#include "weighing/flow_estimator.h"

void setUp(void) {}

void tearDown(void) {}

/** Pours gramsPerSecond for seconds at 10 Hz, from start, and returns the last flow. */
static float pour(FlowEstimator &flow, uint32_t start, float grams, float gramsPerSecond, float seconds)
{
    float result = 0;
    for (int i = 0; i <= seconds * 10; i++)
    {
        result = flow.add(start + i * 100, grams + gramsPerSecond * i / 10.0f);
    }
    return result;
}

void test_constant_flow(void)
{
    FlowEstimator flow;
    TEST_ASSERT_FLOAT_WITHIN(0.001, 2.0, pour(flow, 0, 0, 2, 3));
}

void test_no_flow_from_one_sample(void)
{
    FlowEstimator flow;
    TEST_ASSERT_EQUAL_FLOAT(0, flow.add(0, 100));
}

void test_only_the_window_counts(void)
{
    FlowEstimator flow;
    pour(flow, 0, 0, 5, 3);
    // the cup sits still: 2 s later only the still weight is in the window
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0, pour(flow, 3100, 15, 0, 2));
}

void test_old_samples_drop_out_after_a_gap(void)
{
    FlowEstimator flow;
    pour(flow, 0, 0, 5, 1);
    // nothing for 10 s, e.g. the loop was blocked, then a single sample
    TEST_ASSERT_EQUAL_FLOAT(0, flow.add(11000, 50));
}

void test_same_time_twice_has_no_flow(void)
{
    FlowEstimator flow;
    flow.add(100, 10);
    float result = flow.add(100, 12);
    TEST_ASSERT_FALSE(std::isnan(result));
    TEST_ASSERT_EQUAL_FLOAT(0, result);
}

// Noise like the scale's: 25 counts at 420 counts per gram.
void test_noise_stays_well_below_the_end_of_a_shot(void)
{
    FlowEstimator flow;
    std::mt19937 random(1);
    std::normal_distribution<float> noise(0, 25 / 420.0f);
    float worst = 0;
    for (int i = 0; i < 600; i++)
    {
        float result = flow.add(i * 100, 30 + noise(random));
        if (i >= 20)
        {
            worst = std::fmax(worst, std::fabs(result));
        }
    }
    // shot detection will treat less than 0.1 g/s as stopped
    TEST_ASSERT_LESS_THAN_FLOAT(0.1, worst);
}

void test_precise_after_a_long_uptime(void)
{
    FlowEstimator flow;
    // about 46 days after boot
    TEST_ASSERT_FLOAT_WITHIN(0.001, 2.0, pour(flow, 4000000000u, 250, 2, 3));
}

void test_reset_forgets_the_samples(void)
{
    FlowEstimator flow;
    pour(flow, 0, 0, 2, 1);
    flow.reset();
    TEST_ASSERT_EQUAL_FLOAT(0, flow.add(1100, 2.2f));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_constant_flow);
    RUN_TEST(test_no_flow_from_one_sample);
    RUN_TEST(test_only_the_window_counts);
    RUN_TEST(test_old_samples_drop_out_after_a_gap);
    RUN_TEST(test_same_time_twice_has_no_flow);
    RUN_TEST(test_noise_stays_well_below_the_end_of_a_shot);
    RUN_TEST(test_precise_after_a_long_uptime);
    RUN_TEST(test_reset_forgets_the_samples);
    return UNITY_END();
}
