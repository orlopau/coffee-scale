#include <unity.h>

#include "native/simulated_scale.h"
#include "millis.h"

static SimulatedScale *scale;

void setUp(void)
{
    set_now(1000000);
    scale = new SimulatedScale();
}

void tearDown(void) { delete scale; }

/** Advances the clock in steps of 1 ms, counting the readings like the weight sensor would. */
static int advance(unsigned long ms)
{
    int readings = 0;
    for (unsigned long i = 0; i < ms; i++)
    {
        sleep_for(1);
        if (scale->isReady())
        {
            scale->read();
            readings++;
        }
    }
    return readings;
}

void test_reading_follows_weight(void)
{
    TEST_ASSERT_EQUAL(SimulatedScale::EMPTY_COUNTS, scale->read());
    scale->setWeight(10);
    TEST_ASSERT_EQUAL(SimulatedScale::EMPTY_COUNTS + 10 * SimulatedScale::COUNTS_PER_GRAM, scale->read());
}

void test_weight_never_negative(void)
{
    scale->setWeight(5);
    scale->addWeight(-8);
    TEST_ASSERT_EQUAL_FLOAT(0, scale->getWeight());
}

void test_samples_at_hx711_rate(void)
{
    advance(100); // past the first reading
    TEST_ASSERT_INT_WITHIN(1, SimulatedScale::SAMPLES_PER_SECOND, advance(1000));
}

void test_skips_readings_that_were_not_read(void)
{
    scale->read();
    sleep_for(1000);
    TEST_ASSERT_TRUE(scale->isReady());
    scale->read();
    TEST_ASSERT_FALSE(scale->isReady());
}

void test_flow_fills_cup(void)
{
    scale->setFlow(2);
    advance(5000);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 10, scale->getWeight());

    scale->setFlow(0);
    advance(5000);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 10, scale->getWeight());
}

void test_shot_follows_profile(void)
{
    scale->setWeight(100); // the cup
    scale->startShot();
    TEST_ASSERT_TRUE(scale->isShotRunning());

    advance(5000);
    TEST_ASSERT_EQUAL_FLOAT(100, scale->getWeight()); // pre-infusion

    advance(15000);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 1.6, scale->getFlow());

    advance(15000);
    TEST_ASSERT_FALSE(scale->isShotRunning());
    TEST_ASSERT_EQUAL_FLOAT(0, scale->getFlow());
    TEST_ASSERT_FLOAT_WITHIN(0.1, 138.4, scale->getWeight());
}

void test_shot_ends_with_flow(void)
{
    scale->startShot();
    scale->setFlow(1);
    TEST_ASSERT_FALSE(scale->isShotRunning());
}

void test_noise(void)
{
    scale->setWeight(10);
    scale->setNoise(true);
    long min = scale->read(), max = min;
    for (int i = 0; i < 100; i++)
    {
        long reading = scale->read();
        min = reading < min ? reading : min;
        max = reading > max ? reading : max;
    }
    TEST_ASSERT_GREATER_THAN(SimulatedScale::NOISE_COUNTS, max - min);
    TEST_ASSERT_LESS_THAN(20 * SimulatedScale::NOISE_COUNTS, max - min);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_reading_follows_weight);
    RUN_TEST(test_weight_never_negative);
    RUN_TEST(test_samples_at_hx711_rate);
    RUN_TEST(test_skips_readings_that_were_not_read);
    RUN_TEST(test_flow_fills_cup);
    RUN_TEST(test_shot_follows_profile);
    RUN_TEST(test_shot_ends_with_flow);
    RUN_TEST(test_noise);
    return UNITY_END();
}
