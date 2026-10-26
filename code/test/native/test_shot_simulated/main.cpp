// The emulator's espresso shot (the S key) through the pipeline and the shot
// detector, like the main loop: the pipeline's flow and zero tracking
// included. The shot is accepted, and its pump stop found where the simulated
// pump stops.

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
    int pumpStops = 0;
    /** The first drop, since the pump started. */
    long firstDropMs = 0;
    /** The pump stop, since the pump started. */
    long pumpStopMs = 0;
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
    for (int i = 0; i < 35000; i++)
    {
        sleep_for(1);
        pipeline.update();
        for (const Reading &reading : pipeline.newReadings())
        {
            const ShotEvent event = detector.update(reading);
            if (event == ShotEvent::Accepted)
            {
                result.shots++;
                result.firstDropMs = (long)(detector.firstDropMs() - pumpStart);
                result.lostGrams = (reading.grams - baseline) - reading.net;
            }
            else if (event == ShotEvent::PumpStopped)
            {
                result.pumpStops++;
                result.pumpStopMs = (long)(detector.pumpStopMs() - pumpStart);
            }
        }
    }
    return result;
}

static void assertShot(const Result &result, long firstDropToleranceMs, long pumpStopToleranceMs)
{
    TEST_ASSERT_EQUAL(1, result.shots);
    TEST_ASSERT_EQUAL(1, result.pumpStops);
    // The simulated flow rises from 0 at 6 s to 1.6 g/s at 9 s, so the weight
    // rises quadratically: 0.07 g after 0.5 s, 0.27 g after 1 s. Below 0.1 g
    // the weight doesn't measurably leave rest, the first drop is seen at 6.5 s.
    // The pump stops at 30 s.
    TEST_ASSERT_INT_WITHIN(firstDropToleranceMs, 6500, result.firstDropMs);
    TEST_ASSERT_INT_WITHIN(pumpStopToleranceMs, 30000, result.pumpStopMs);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0, result.lostGrams);
}

void setUp(void) {}

void tearDown(void) {}

void test_the_shot_is_accepted_and_its_pump_stop_found(void) { assertShot(runShot(false), 500, 500); }

void test_with_noise(void) { assertShot(runShot(true), 1000, 500); }

void test_zero_tracking_takes_little(void) { TEST_ASSERT_FLOAT_WITHIN(0.1f, 0, runShot(false).lostGrams); }

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_the_shot_is_accepted_and_its_pump_stop_found);
    RUN_TEST(test_with_noise);
    RUN_TEST(test_zero_tracking_takes_little);
    UNITY_END();
}
