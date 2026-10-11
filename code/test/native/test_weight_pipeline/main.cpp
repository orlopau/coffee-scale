#include <unity.h>

#include "mock/mock_sample_source.h"
#include "weighing/weight_pipeline.h"

static QueuedSamples *samples;
static WeightPipeline *pipeline;

void setUp(void)
{
    samples = new QueuedSamples();
    pipeline = new WeightPipeline(*samples);
    pipeline->setGramsPerCount(0.01f);
}

void tearDown(void)
{
    delete pipeline;
    delete samples;
}

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

void test_no_readings_without_samples(void)
{
    pipeline->setGramsPerCount(1);
    pipeline->update();
    TEST_ASSERT_TRUE(pipeline->newReadings().empty());
    TEST_ASSERT_EQUAL_FLOAT(0, pipeline->getWeight());
}

void test_all_waiting_samples_become_readings_in_order(void)
{
    pipeline->setGramsPerCount(1);
    samples->samples = {{100, 10}, {200, 20}, {300, 30}};
    pipeline->update();

    const std::vector<Reading> &readings = pipeline->newReadings();
    TEST_ASSERT_EQUAL(3, readings.size());
    TEST_ASSERT_EQUAL(100, readings[0].ms);
    TEST_ASSERT_EQUAL(20, readings[1].raw);
    TEST_ASSERT_EQUAL(300, readings[2].ms);
    TEST_ASSERT_EQUAL_FLOAT(30, pipeline->getWeight());
}

void test_readings_are_new_only_once(void)
{
    pipeline->setGramsPerCount(1);
    samples->samples = {{100, 10}};
    pipeline->update();
    pipeline->update();

    TEST_ASSERT_TRUE(pipeline->newReadings().empty());
    TEST_ASSERT_EQUAL_FLOAT(10, pipeline->getWeight());
}

void test_weight_is_calibrated_and_tared(void)
{
    pipeline->setGramsPerCount(0.5f);
    samples->samples = {{100, 1000}};
    pipeline->update();
    TEST_ASSERT_EQUAL_FLOAT(500, pipeline->getWeight());

    pipeline->tare();
    TEST_ASSERT_EQUAL_FLOAT(0, pipeline->getWeight());

    samples->samples = {{200, 1010}};
    pipeline->update();
    TEST_ASSERT_EQUAL_FLOAT(5, pipeline->getWeight());
    TEST_ASSERT_EQUAL_FLOAT(505, pipeline->newReadings()[0].grams);
}

void test_auto_averaging_threshold_in_grams_per_second(void)
{
    pipeline->setGramsPerCount(0.01f);
    // 0.1 g in 100 ms is 1 g/s, under 2 g/s, so averaged
    pipeline->setAutoAveraging(2, 4);
    samples->samples = {{0, 10000}, {100, 10000}, {200, 10010}};
    pipeline->update();
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 100.05, pipeline->getWeight());
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
    RUN_TEST(test_no_readings_without_samples);
    RUN_TEST(test_all_waiting_samples_become_readings_in_order);
    RUN_TEST(test_readings_are_new_only_once);
    RUN_TEST(test_weight_is_calibrated_and_tared);
    RUN_TEST(test_auto_averaging_threshold_in_grams_per_second);
    return UNITY_END();
}
