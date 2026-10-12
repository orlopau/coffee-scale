#include <unity.h>

#include <cmath>
#include <filesystem>

#include "millis.h"
#include "native/recorded_samples.h"
#include "native/recorded_scale.h"
#include "weighing/weight_pipeline.h"

namespace fs = std::filesystem;

static const fs::path FIXTURES = fs::path(__FILE__).parent_path() / "fixtures";
static const fs::path RECORDINGS = fs::path(__FILE__).parent_path().parent_path().parent_path() / "recordings";

void setUp(void) { set_now(1000000); }

void tearDown(void) {}

void test_pour_of_2_grams_per_second(void)
{
    RecordedScale scale((FIXTURES / "pour_2gs.csv").string());
    RecordedSamples samples(scale);
    WeightPipeline pipeline(samples);
    pipeline.setGramsPerCount(scale.gramsPerCount());

    // the empty cup for 1 s, then tare
    scale.update();
    for (int i = 0; i < 10; i++)
    {
        pipeline.update();
        sleep_for(100);
    }
    pipeline.tare();

    // 4 s of pouring
    for (int i = 0; i <= 40; i++)
    {
        pipeline.update();
        sleep_for(100);
    }

    const Reading &reading = pipeline.newReadings().back();
    TEST_ASSERT_FLOAT_WITHIN(0.05, 2, reading.flow);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 8, reading.net);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 108, reading.grams);
}

/**
 * Replays the cold start of the empty scale through a pipeline set up like at
 * start-up: tared after 32 readings, then every reading until the end. Calls
 * check(elapsed ms, filtered) for each reading after the tare.
 */
template <typename Check>
static void replayWarmup(float zeroTrackingWindow, Check check,
                         int32_t beanCounts = 0, unsigned long beanFromMs = 0)
{
    RecordedScale scale((RECORDINGS / "warmup.csv").string());
    RecordedSamples samples(scale);
    if (beanCounts != 0)
    {
        samples.add(beanCounts, beanFromMs);
    }
    WeightPipeline pipeline(samples);
    pipeline.setGramsPerCount(scale.gramsPerCount());
    // the averaging threshold's default setting
    pipeline.setAutoAveraging(1.0f, 64);
    pipeline.setZeroTracking(zeroTrackingWindow);

    scale.update();
    int readings = 0;
    while (readings < 32)
    {
        pipeline.update();
        readings += pipeline.newReadings().size();
        sleep_for(10);
    }
    pipeline.tare();

    while (!scale.isFinished())
    {
        pipeline.update();
        for (const Reading &reading : pipeline.newReadings())
        {
            check(scale.elapsed(), reading.filtered);
        }
        sleep_for(10);
    }
}

// Guards the tests below: without tracking, the recording drifts far from zero.
void test_warmup_drifts_without_zero_tracking(void)
{
    float last = 0;
    replayWarmup(0, [&](unsigned long, float filtered) { last = filtered; });
    TEST_ASSERT_TRUE(fabsf(last) > 0.3f);
}

void test_warmup_reads_zero_with_zero_tracking(void)
{
    float worst = 0;
    replayWarmup(0.1f, [&](unsigned long, float filtered) { worst = fmaxf(worst, fabsf(filtered)); });
    // rounds to 0.00 on the display
    TEST_ASSERT_FLOAT_WITHIN(0.005, 0, worst);
}

// A 0.13 g bean, placed on the warm scale for the last minute of the recording.
void test_a_bean_stays_on_the_display(void)
{
    RecordedScale scale((RECORDINGS / "warmup.csv").string());
    const int32_t beanCounts = lroundf(0.13f / scale.gramsPerCount());
    const unsigned long placed = 220000;

    float lightest = 1000;
    replayWarmup(0.1f, [&](unsigned long ms, float filtered) {
        // once the averaging has caught up with the bean
        if (ms > placed + 1000)
        {
            lightest = fminf(lightest, filtered);
        }
    }, beanCounts, placed);
    TEST_ASSERT_TRUE(lightest >= 0.105f);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_pour_of_2_grams_per_second);
    RUN_TEST(test_warmup_drifts_without_zero_tracking);
    RUN_TEST(test_warmup_reads_zero_with_zero_tracking);
    RUN_TEST(test_a_bean_stays_on_the_display);
    return UNITY_END();
}
