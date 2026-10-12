#include <unity.h>

#include <initializer_list>

#include "detectors/lift_tare.h"
#include "mock/mock_interface.h"
#include "mock/mock_sample_source.h"
#include "weighing/weight_pipeline.h"

static QueuedSamples *samples;
static WeightPipeline *pipeline;
static LiftTare *liftTare;
static uint32_t ms;

void setUp(void)
{
    Interface::reset();
    samples = new QueuedSamples();
    pipeline = new WeightPipeline(*samples);
    pipeline->setGramsPerCount(0.01f);
    liftTare = new LiftTare(*pipeline);
    liftTare->setDepth(0.7f);
    ms = 1000;
}

void tearDown(void)
{
    delete liftTare;
    delete pipeline;
    delete samples;
}

/** Queues readings 90 ms apart, in grams. */
static void queue(std::initializer_list<float> grams)
{
    for (float g : grams)
    {
        ms += 90;
        samples->samples.push_back({ms, static_cast<int32_t>(g * 100)});
    }
}

/** One loop: the pipeline takes the queued samples, then lift-to-tare looks at them. */
static void loop(bool measuring)
{
    pipeline->update();
    liftTare->update(measuring);
}

void test_a_lift_tares_and_beeps_twice(void)
{
    queue({100, 100, 100, 100, 100, 98.5f, 100});
    loop(false);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0, pipeline->latest().net);
    TEST_ASSERT_EQUAL(2, Interface::buzzerBeeps);
}

void test_no_tare_while_measuring(void)
{
    queue({100, 100, 100, 100, 100, 98.5f, 100});
    loop(true);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 100, pipeline->latest().net);
    TEST_ASSERT_EQUAL(0, Interface::buzzerBeeps);
}

void test_a_dip_from_while_measuring_does_not_count(void)
{
    queue({100, 100, 100, 100, 100, 98.5f});
    loop(true);
    queue({100});
    loop(false);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 100, pipeline->latest().net);
    TEST_ASSERT_EQUAL(0, Interface::buzzerBeeps);
}

void test_off_never_tares(void)
{
    liftTare->setDepth(0);
    queue({100, 100, 100, 100, 100, 98.5f, 100});
    loop(false);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 100, pipeline->latest().net);
    TEST_ASSERT_EQUAL(0, Interface::buzzerBeeps);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_a_lift_tares_and_beeps_twice);
    RUN_TEST(test_no_tare_while_measuring);
    RUN_TEST(test_a_dip_from_while_measuring_does_not_count);
    RUN_TEST(test_off_never_tares);
    return UNITY_END();
}
