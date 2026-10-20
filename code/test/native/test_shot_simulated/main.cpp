// The emulator's espresso shot (the S key) through the pipeline and the shot
// detector, like the main loop: the pipeline's flow and zero tracking
// included, until the real shot recordings are in.

#include <unity.h>

#include <math.h>

#include "detectors/shot_detector.h"
#include "millis.h"
#include "native/simulated_scale.h"
#include "weighing/weight_pipeline.h"

class SimulatedSamples : public SampleSource
{
public:
    explicit SimulatedSamples(SimulatedScale &scale) : scale(scale) {}

    bool next(Sample &sample) override
    {
        scale.update();
        if (!scale.isReady())
        {
            return false;
        }
        sample = {static_cast<uint32_t>(now()), static_cast<int32_t>(scale.read())};
        return true;
    }

private:
    SimulatedScale &scale;
};

struct Result
{
    int shots = 0;
    /** The first drop, since the pump started. */
    long firstDropMs = 0;
    /** What zero tracking took from the shot's weight by the time it was found. */
    float lostGrams = 0;
};

static Result runShot(bool noise)
{
    set_now(1000);
    SimulatedScale scale;
    scale.setNoise(noise);
    SimulatedSamples samples(scale);
    WeightPipeline pipeline(samples);
    pipeline.setGramsPerCount(1 / SimulatedScale::COUNTS_PER_GRAM);
    pipeline.setAutoAveraging(1.0f, 64);
    pipeline.setZeroTracking(0.1f);
    ShotDetector detector;
    // 36 g in 30 s
    detector.setExpectedFlow(1.2f);

    // a cup, tared once it settled
    scale.setWeight(150);
    for (int i = 0; i < 3000; i++)
    {
        sleep_for(1);
        pipeline.update();
    }
    pipeline.tare();
    float grams = 0;
    int count = 0;
    for (int i = 0; i < 2000; i++)
    {
        sleep_for(1);
        pipeline.update();
        for (const Reading &reading : pipeline.newReadings())
        {
            grams += reading.grams;
            count++;
        }
    }
    const float baseline = grams / count;

    Result result;
    const unsigned long pumpStart = now();
    scale.startShot();
    for (int i = 0; i < 20000; i++)
    {
        sleep_for(1);
        pipeline.update();
        for (const Reading &reading : pipeline.newReadings())
        {
            if (detector.update(reading))
            {
                result.shots++;
                result.firstDropMs = (long)(detector.firstDropMs() - pumpStart);
                result.lostGrams = (reading.grams - baseline) - reading.net;
            }
        }
    }
    return result;
}

static void assertShot(const Result &result)
{
    TEST_ASSERT_EQUAL(1, result.shots);
    // the simulated drops begin at 6 s and reach full flow at 9 s
    TEST_ASSERT_GREATER_OR_EQUAL(6000, result.firstDropMs);
    TEST_ASSERT_LESS_OR_EQUAL(9000, result.firstDropMs);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0, result.lostGrams);
}

void setUp(void) {}

void tearDown(void) {}

void test_the_simulated_shot_is_found(void) { assertShot(runShot(false)); }

void test_the_simulated_shot_is_found_with_noise(void) { assertShot(runShot(true)); }

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_the_simulated_shot_is_found);
    RUN_TEST(test_the_simulated_shot_is_found_with_noise);
    UNITY_END();
}
