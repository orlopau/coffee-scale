#include <unity.h>

#include "mock/mock_sample_source.h"
#include "weighing/weight_sensor.h"

static QueuedSamples *samples;
static DefaultWeightSensor *weightSensor;

void setUp(void)
{
    samples = new QueuedSamples();
    weightSensor = new DefaultWeightSensor(*samples);
}

void tearDown(void)
{
    delete weightSensor;
    delete samples;
}

void test_no_readings_without_samples(void)
{
    weightSensor->update();
    TEST_ASSERT_TRUE(weightSensor->newReadings().empty());
    TEST_ASSERT_EQUAL_FLOAT(0, weightSensor->getWeight());
}

void test_all_waiting_samples_become_readings_in_order(void)
{
    samples->samples = {{100, 10}, {200, 20}, {300, 30}};
    weightSensor->update();

    const std::vector<Reading> &readings = weightSensor->newReadings();
    TEST_ASSERT_EQUAL(3, readings.size());
    TEST_ASSERT_EQUAL(100, readings[0].ms);
    TEST_ASSERT_EQUAL(20, readings[1].raw);
    TEST_ASSERT_EQUAL(300, readings[2].ms);
    TEST_ASSERT_EQUAL_FLOAT(30, weightSensor->getWeight());
}

void test_readings_are_new_only_once(void)
{
    samples->samples = {{100, 10}};
    weightSensor->update();
    weightSensor->update();

    TEST_ASSERT_TRUE(weightSensor->newReadings().empty());
    TEST_ASSERT_EQUAL_FLOAT(10, weightSensor->getWeight());
}

void test_weight_is_calibrated_and_tared(void)
{
    weightSensor->setScale(0.5f);
    samples->samples = {{100, 1000}};
    weightSensor->update();
    TEST_ASSERT_EQUAL_FLOAT(500, weightSensor->getWeight());

    weightSensor->tare();
    TEST_ASSERT_EQUAL_FLOAT(0, weightSensor->getWeight());

    samples->samples = {{200, 1010}};
    weightSensor->update();
    TEST_ASSERT_EQUAL_FLOAT(5, weightSensor->getWeight());
    TEST_ASSERT_EQUAL_FLOAT(505, weightSensor->newReadings()[0].grams);
}

void test_auto_averaging_threshold_in_grams_per_second(void)
{
    weightSensor->setScale(0.01f);
    // 0.1 g in 100 ms is 1 g/s, under 2 g/s, so averaged
    weightSensor->setAutoAveraging(2, 4);
    samples->samples = {{0, 10000}, {100, 10000}, {200, 10010}};
    weightSensor->update();
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 100.05, weightSensor->getWeight());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_no_readings_without_samples);
    RUN_TEST(test_all_waiting_samples_become_readings_in_order);
    RUN_TEST(test_readings_are_new_only_once);
    RUN_TEST(test_weight_is_calibrated_and_tared);
    RUN_TEST(test_auto_averaging_threshold_in_grams_per_second);
    return UNITY_END();
}
