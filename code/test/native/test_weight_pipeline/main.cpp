#include <unity.h>

#include "weighing/weight_pipeline.h"

static WeightPipeline *pipeline;

void setUp(void)
{
    pipeline = new WeightPipeline();
    pipeline->setGramsPerCount(0.01f);
}

void tearDown(void) { delete pipeline; }

void test_no_reading_before_the_first_sample(void) { TEST_ASSERT_FALSE(pipeline->hasReading()); }

void test_calibrates_the_raw_value(void)
{
    Reading r = pipeline->process({500, 1234});
    TEST_ASSERT_EQUAL(500, r.ms);
    TEST_ASSERT_EQUAL(1234, r.raw);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 12.34, r.grams);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 12.34, r.net);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 12.34, r.filtered);
    TEST_ASSERT_TRUE(pipeline->hasReading());
    TEST_ASSERT_EQUAL(1234, pipeline->latest().raw);
}

void test_tare_zeroes_net_and_filtered_but_not_grams(void)
{
    pipeline->process({0, 10000});
    pipeline->tare();
    Reading r = pipeline->process({100, 10050});
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 100.5, r.grams);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 0.5, r.net);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 0.5, r.filtered);
}

void test_tare_takes_the_averaged_weight(void)
{
    pipeline->setAutoAveraging(1, 4);
    pipeline->process({0, 10000});
    pipeline->process({100, 10002});
    pipeline->process({200, 9998});
    pipeline->tare();
    // offset is the average 100.0 g, not the last 99.98 g
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 0, pipeline->latest().filtered);
    Reading r = pipeline->process({300, 10000});
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 0, r.net);
}

void test_filtered_is_averaged_net_is_not(void)
{
    pipeline->setAutoAveraging(1, 4);
    pipeline->process({0, 10000});
    pipeline->process({100, 10002});
    Reading r = pipeline->process({200, 9998});
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 99.98, r.net);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 100.0, r.filtered);
}

void test_tare_does_not_disturb_the_flow(void)
{
    Reading r;
    for (int i = 0; i <= 20; i++)
    {
        r = pipeline->process({(uint32_t)(i * 100), 10000 + i * 20});
    }
    TEST_ASSERT_FLOAT_WITHIN(0.001, 2, r.flow);
    pipeline->tare();
    r = pipeline->process({2100, 10000 + 21 * 20});
    TEST_ASSERT_FLOAT_WITHIN(0.001, 2, r.flow);
}

void test_tare_before_the_first_sample_does_nothing(void)
{
    pipeline->tare();
    Reading r = pipeline->process({0, 500});
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 5, r.net);
}

// Calibrating sets a new scale after the start-up tare, which must still hold.
void test_tare_survives_a_new_scale(void)
{
    pipeline->setGramsPerCount(1);
    pipeline->setAutoAveraging(1, 4);
    pipeline->process({0, 84000});
    pipeline->tare();

    pipeline->setGramsPerCount(0.002f);
    Reading r = pipeline->process({100, 134000});
    TEST_ASSERT_FLOAT_WITHIN(0.001, 100, r.net);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 100, r.filtered);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 268, r.grams);
}

// The filter and flow hold grams of the old scale, which mean nothing in the new one.
void test_new_scale_starts_filter_and_flow_over(void)
{
    pipeline->setGramsPerCount(1);
    pipeline->setAutoAveraging(1000, 4);
    for (int i = 0; i < 10; i++)
    {
        pipeline->process({(uint32_t)(i * 100), 84000});
    }

    pipeline->setGramsPerCount(0.002f);
    Reading r = pipeline->process({1000, 84000});
    TEST_ASSERT_FLOAT_WITHIN(0.001, 168, r.filtered);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0, r.flow);
}

void test_tare_right_after_a_new_scale_uses_the_last_sample(void)
{
    pipeline->setGramsPerCount(1);
    pipeline->process({0, 84000});

    pipeline->setGramsPerCount(0.002f);
    pipeline->tare();
    Reading r = pipeline->process({100, 84000});
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0, r.net);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_no_reading_before_the_first_sample);
    RUN_TEST(test_calibrates_the_raw_value);
    RUN_TEST(test_tare_zeroes_net_and_filtered_but_not_grams);
    RUN_TEST(test_tare_takes_the_averaged_weight);
    RUN_TEST(test_filtered_is_averaged_net_is_not);
    RUN_TEST(test_tare_does_not_disturb_the_flow);
    RUN_TEST(test_tare_before_the_first_sample_does_nothing);
    RUN_TEST(test_tare_survives_a_new_scale);
    RUN_TEST(test_new_scale_starts_filter_and_flow_over);
    RUN_TEST(test_tare_right_after_a_new_scale_uses_the_last_sample);
    return UNITY_END();
}
