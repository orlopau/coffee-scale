// The recordings of ADR 1 (docs/adr/0001-tare-gesture.md) through the
// pipeline and the lift detector, like the main loop: every lift must tare,
// nothing else may.

#include <unity.h>

#include <filesystem>
#include <vector>

#include "detectors/lift_detector.h"
#include "millis.h"
#include "native/recorded_samples.h"
#include "native/recorded_scale.h"
#include "weighing/weight_pipeline.h"

namespace fs = std::filesystem;

static const fs::path RECORDINGS = fs::path(__FILE__).parent_path().parent_path().parent_path() / "recordings";

// the levels of the setting
static const float GENTLE = 0.5f;
static const float NORMAL = 0.7f;
static const float FIRM = 1.5f;

struct Lift
{
    /** When it was reported, in the recording's ms. */
    unsigned long ms;
    /** The mean net of the three readings after the tare. */
    float netAfter;
};

static std::vector<Lift> replay(const char *recording, float depth)
{
    set_now(1000000);
    RecordedScale scale((RECORDINGS / recording).string());
    RecordedSamples samples(scale);
    WeightPipeline pipeline(samples);
    pipeline.setGramsPerCount(scale.gramsPerCount());
    pipeline.setAutoAveraging(1.0f, 64);
    LiftDetector detector;
    detector.setDepth(depth);

    std::vector<Lift> lifts;
    int netsToAdd = 0;
    scale.update();
    while (!scale.isFinished())
    {
        pipeline.update();
        for (const Reading &reading : pipeline.newReadings())
        {
            if (netsToAdd > 0)
            {
                lifts.back().netAfter += reading.net / 3;
                netsToAdd--;
            }
            float weightBefore;
            if (detector.update(reading.ms, reading.grams, weightBefore))
            {
                pipeline.tareTo(weightBefore);
                lifts.push_back({scale.elapsed(), 0});
                netsToAdd = 3;
            }
        }
        sleep_for(1);
    }
    return lifts;
}

static void assertLifts(const std::vector<unsigned long> &expected, const std::vector<Lift> &lifts)
{
    TEST_ASSERT_EQUAL(expected.size(), lifts.size());
    for (size_t i = 0; i < lifts.size(); i++)
    {
        TEST_ASSERT_UINT32_WITHIN(5, expected[i], lifts[i].ms);
        // the platform doesn't always settle exactly where it was, see the spec
        TEST_ASSERT_FLOAT_WITHIN(0.12f, 0, lifts[i].netAfter);
    }
}

static const char *LIFTS_EMPTY = "2026-10-11_23-13-38.csv";
static const char *LIFTS_WITH_CUP = "2026-10-11_23-16-30.csv";

void test_lifts_on_the_empty_scale(void)
{
    const std::vector<unsigned long> all = {1148, 2771, 4213, 5745, 7007, 8359, 9351, 10432, 11424, 12415};
    assertLifts(all, replay(LIFTS_EMPTY, GENTLE));
    assertLifts(all, replay(LIFTS_EMPTY, NORMAL));
    // the two shallowest, 0.94 and 0.96 g
    assertLifts({1148, 2771, 4213, 5745, 8359, 10432, 11424, 12415}, replay(LIFTS_EMPTY, FIRM));
}

// also the cup taken off and put back, the pour and the button presses for the marks: none of them is a lift
void test_lifts_with_a_cup(void)
{
    const std::vector<unsigned long> all = {4033, 5024, 6016, 7007, 7998, 8810, 9711, 10522, 11153, 12145, 12956, 13677};
    assertLifts(all, replay(LIFTS_WITH_CUP, GENTLE));
    assertLifts(all, replay(LIFTS_WITH_CUP, NORMAL));
    // the shallowest, 1.44 g
    assertLifts({4033, 5024, 6016, 7007, 7998, 8810, 9711, 11153, 12145, 12956, 13677}, replay(LIFTS_WITH_CUP, FIRM));
}

void test_taps_cups_bumps_and_the_warmup_are_no_lifts(void)
{
    for (const char *recording : {"2026-10-11_23-03-07.csv", "2026-10-11_23-07-55.csv", "warmup.csv"})
    {
        for (float depth : {GENTLE, NORMAL, FIRM})
        {
            TEST_ASSERT_EQUAL_MESSAGE(0, replay(recording, depth).size(), recording);
        }
    }
}

void setUp(void) {}

void tearDown(void) {}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_lifts_on_the_empty_scale);
    RUN_TEST(test_lifts_with_a_cup);
    RUN_TEST(test_taps_cups_bumps_and_the_warmup_are_no_lifts);
    return UNITY_END();
}
