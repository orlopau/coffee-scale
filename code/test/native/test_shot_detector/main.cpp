#include <unity.h>

#include "detectors/shot_detector.h"

static ShotDetector *detector;
static uint32_t ms;
/** Reports of update() so far, and the time of the last one. */
static int reports;
static uint32_t reportMs;
/** Time of the first reading of the last rise(). */
static uint32_t firstRiseMs;

// 36 g in 30 s
static const float EXPECTED_FLOW = 1.2f;

void setUp(void)
{
    detector = new ShotDetector();
    detector->setExpectedFlow(EXPECTED_FLOW);
    ms = 1000;
    reports = 0;
    reportMs = 0;
    firstRiseMs = 0;
}

void tearDown(void) { delete detector; }

/** One reading 100 ms after the last. */
static void feed(float grams, float net, float flow = 0)
{
    ms += 100;
    if (detector->update({ms, 0, grams, net, net, flow}))
    {
        reports++;
        reportMs = ms;
    }
}

static void steady(int readings, float grams = 100, float net = 0)
{
    for (int i = 0; i < readings; i++)
    {
        feed(grams, net);
    }
}

/** Liquid flowing in from 0.4 g above fromGrams on, like the first drops, for seconds. */
static void rise(float seconds, float gramsPerSecond, float fromGrams = 100, float fromNet = 0)
{
    const int readings = (int)(seconds * 10);
    for (int i = 0; i < readings; i++)
    {
        const float added = 0.4f + gramsPerSecond * 0.1f * i;
        feed(fromGrams + added, fromNet + added, gramsPerSecond);
        if (i == 0)
        {
            firstRiseMs = ms;
        }
    }
}

void test_a_steady_rise_is_a_shot(void)
{
    steady(5);
    rise(4, EXPECTED_FLOW);
    TEST_ASSERT_EQUAL(1, reports);
    TEST_ASSERT_EQUAL(firstRiseMs + ShotDetector::CONFIRM_MS, reportMs);
    TEST_ASSERT_EQUAL(firstRiseMs, detector->firstDropMs());
    TEST_ASSERT_EQUAL(firstRiseMs, detector->rise().front().ms);
    TEST_ASSERT_EQUAL(reportMs, detector->rise().back().ms);
}

void test_a_shot_on_a_tared_cup_is_found(void)
{
    steady(5, 155.9f, 0);
    rise(4, EXPECTED_FLOW, 155.9f);
    TEST_ASSERT_EQUAL(1, reports);
}

// e.g. sliding the scale under the portafilter
void test_bumps_are_not_a_shot(void)
{
    steady(5);
    feed(101.5f, 1.5f);
    feed(100, 0);
    feed(100.8f, 0.8f);
    feed(100, 0);
    steady(5);
    TEST_ASSERT_EQUAL(0, reports);

    rise(4, EXPECTED_FLOW);
    TEST_ASSERT_EQUAL(1, reports);
}

// e.g. pouring water
void test_a_fast_rise_is_not_a_shot(void)
{
    steady(5);
    rise(4, EXPECTED_FLOW * ShotDetector::MAX_FLOW_FACTOR * 1.5f);
    TEST_ASSERT_EQUAL(0, reports);
}

void test_a_fast_rise_is_a_shot_with_a_higher_expected_flow(void)
{
    // a turbo: 36 g in 15 s
    detector->setExpectedFlow(2.4f);
    steady(5);
    rise(4, EXPECTED_FLOW * ShotDetector::MAX_FLOW_FACTOR * 1.5f);
    TEST_ASSERT_EQUAL(1, reports);
}

void test_a_slow_rise_is_not_a_shot(void)
{
    steady(5);
    rise(4, EXPECTED_FLOW * ShotDetector::MIN_FLOW_FACTOR * 0.5f);
    TEST_ASSERT_EQUAL(0, reports);
}

// e.g. a spoon set into the cup: over 2 s it looks like 1 g/s, but nothing flows
void test_a_step_that_stays_flat_is_not_a_shot(void)
{
    steady(5);
    for (uint32_t t = 0; t < ShotDetector::CONFIRM_MS + 500; t += 100)
    {
        feed(102, 2, 0);
    }
    TEST_ASSERT_EQUAL(0, reports);
}

// e.g. the drips after a shot, 36 g in the cup
void test_not_armed_away_from_zero(void)
{
    steady(5, 100, 36);
    rise(4, EXPECTED_FLOW, 100, 36);
    TEST_ASSERT_EQUAL(0, reports);

    // tared again
    steady(5, 100, 0);
    rise(4, EXPECTED_FLOW);
    TEST_ASSERT_EQUAL(1, reports);
}

void test_one_report_per_tare(void)
{
    steady(5);
    rise(12, EXPECTED_FLOW);
    TEST_ASSERT_EQUAL(1, reports);
}

void test_reset_forgets_a_rise(void)
{
    steady(5);
    for (int i = 0; i < 40; i++)
    {
        if (i == 10)
        {
            detector->reset();
        }
        const float added = 0.4f + EXPECTED_FLOW * 0.1f * i;
        feed(100 + added, added, EXPECTED_FLOW);
    }
    TEST_ASSERT_EQUAL(0, reports);
}

void test_a_gap_resets(void)
{
    steady(5);
    for (int i = 0; i < 40; i++)
    {
        if (i == 10)
        {
            ms += ShotDetector::MAX_GAP_MS;
        }
        const float added = 0.4f + EXPECTED_FLOW * 0.1f * i;
        feed(100 + added, added, EXPECTED_FLOW);
    }
    TEST_ASSERT_EQUAL(0, reports);
}

// a slow trickle must not drag the baseline along until it never counts as a rise
void test_a_slow_first_trickle_still_begins_a_rise(void)
{
    steady(5);
    const float flow = EXPECTED_FLOW * ShotDetector::MIN_FLOW_FACTOR * 1.5f;
    // from the baseline on, without a first jump
    for (int i = 1; i <= 60; i++)
    {
        feed(100 + flow * 0.1f * i, flow * 0.1f * i, flow);
    }
    TEST_ASSERT_EQUAL(1, reports);
}

// noise on the first drops dips one reading under the rise threshold, but not back to the baseline
void test_noise_on_the_first_drops_is_not_a_bump(void)
{
    steady(5);
    rise(0.3f, EXPECTED_FLOW);
    const float dip = (ShotDetector::RISE_GRAMS + ShotDetector::BACK_GRAMS) / 2;
    feed(100 + dip, dip, EXPECTED_FLOW);
    rise(3, EXPECTED_FLOW, 100.4f);
    TEST_ASSERT_EQUAL(1, reports);
}

// only net changes with a tare, the detector watches grams
void test_a_tare_during_the_steady_stretch_changes_nothing(void)
{
    steady(5, 100, 2);
    steady(5, 100, 0);
    rise(4, EXPECTED_FLOW);
    TEST_ASSERT_EQUAL(1, reports);
}

// e.g. a vibration pump shaking the tray as the drops begin: back at the baseline, still armed
void test_a_bump_keeps_the_detector_armed(void)
{
    steady(5);
    feed(100.8f, 0.8f);
    feed(100, 0);
    rise(4, EXPECTED_FLOW);
    TEST_ASSERT_EQUAL(1, reports);
}

// a long preinfusion trickles too slowly at first, then flows
void test_a_slow_start_is_found_once_it_flows(void)
{
    steady(5);
    const float slow = EXPECTED_FLOW * ShotDetector::MIN_FLOW_FACTOR * 0.5f;
    rise(3, slow);
    const uint32_t firstDrop = firstRiseMs;
    const float trickled = 0.4f + slow * 0.1f * 29;
    rise(4, EXPECTED_FLOW * 1.5f, 100 + trickled, trickled);
    TEST_ASSERT_EQUAL(1, reports);
    TEST_ASSERT_EQUAL(firstDrop, detector->firstDropMs());
}

// pins that a rise is given up: it could be anything after that long
void test_a_rise_that_never_flows_enough_is_given_up(void)
{
    steady(5);
    const float slow = EXPECTED_FLOW * ShotDetector::MIN_FLOW_FACTOR * 0.5f;
    const float seconds = ShotDetector::MAX_RISE_MS / 1000.0f + 1;
    rise(seconds, slow);
    const float trickled = 0.4f + slow * seconds;
    rise(4, EXPECTED_FLOW, 100 + trickled, trickled);
    TEST_ASSERT_EQUAL(0, reports);
}

// the first drops weigh less than RISE_GRAMS, the shot began where the weight left the baseline
void test_the_first_drop_is_where_the_weight_left_the_baseline(void)
{
    steady(5);
    const float flow = EXPECTED_FLOW / 2;
    uint32_t firstDrop = 0;
    for (int i = 1; i <= 40; i++)
    {
        const float added = flow * 0.1f * i;
        feed(100 + added, added, flow);
        if (firstDrop == 0 && added > ShotDetector::DROP_GRAMS)
        {
            firstDrop = ms;
        }
    }
    TEST_ASSERT_EQUAL(1, reports);
    TEST_ASSERT_EQUAL(firstDrop, detector->firstDropMs());
    TEST_ASSERT_EQUAL(firstDrop, detector->rise().front().ms);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_a_steady_rise_is_a_shot);
    RUN_TEST(test_a_shot_on_a_tared_cup_is_found);
    RUN_TEST(test_bumps_are_not_a_shot);
    RUN_TEST(test_a_fast_rise_is_not_a_shot);
    RUN_TEST(test_a_fast_rise_is_a_shot_with_a_higher_expected_flow);
    RUN_TEST(test_a_slow_rise_is_not_a_shot);
    RUN_TEST(test_a_step_that_stays_flat_is_not_a_shot);
    RUN_TEST(test_not_armed_away_from_zero);
    RUN_TEST(test_one_report_per_tare);
    RUN_TEST(test_reset_forgets_a_rise);
    RUN_TEST(test_a_gap_resets);
    RUN_TEST(test_a_slow_first_trickle_still_begins_a_rise);
    RUN_TEST(test_noise_on_the_first_drops_is_not_a_bump);
    RUN_TEST(test_a_tare_during_the_steady_stretch_changes_nothing);
    RUN_TEST(test_a_bump_keeps_the_detector_armed);
    RUN_TEST(test_a_slow_start_is_found_once_it_flows);
    RUN_TEST(test_a_rise_that_never_flows_enough_is_given_up);
    RUN_TEST(test_the_first_drop_is_where_the_weight_left_the_baseline);
    UNITY_END();
}
