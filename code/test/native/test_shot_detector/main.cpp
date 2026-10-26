#include <math.h>
#include <random>
#include <unity.h>
#include <vector>

#include "detectors/shot_detector.h"

/** The cup on the scale, before the tare. */
static const float CUP_GRAMS = 100;
static const uint32_t INTERVAL_MS = 90;

struct Event
{
    ShotEvent event;
    uint32_t ms;
};

static ShotDetector *detector;
/** Time of the last reading. */
static uint32_t ms;
/** What the scale weighed at the last tare. */
static float tareGrams;
static std::mt19937 noise;
/** One draw sequence across all feeds of a test, scaled by each feed's noise. */
static std::normal_distribution<float> unitNoise(0, 1);
static std::vector<Event> events;

void setUp(void)
{
    detector = new ShotDetector();
    ms = 1000;
    tareGrams = 0;
    noise.seed(42);
    unitNoise.reset();
    events.clear();
}

void tearDown(void) { delete detector; }

/**
 * Readings every 90 ms for durationMs: grams is the cup plus gramsAt(t), t the
 * seconds since the first of these readings, plus noise of noiseSigma.
 */
template <typename Profile> static void feed(uint32_t durationMs, Profile gramsAt, float noiseSigma = 0)
{
    for (uint32_t t = 0; t < durationMs; t += INTERVAL_MS)
    {
        ms += INTERVAL_MS;
        const float exact = CUP_GRAMS + gramsAt(t / 1000.0f);
        const float grams = noiseSigma > 0 ? exact + noiseSigma * unitNoise(noise) : exact;
        const ShotEvent event = detector->update({ms, 0, grams, grams - tareGrams, 0, 0});
        if (event != ShotEvent::None)
        {
            events.push_back({event, ms});
        }
    }
}

/** The cup tared with inCup grams in it: the profiles are what is in it. */
static void tare(float inCup = 0) { tareGrams = CUP_GRAMS + inCup; }

/** The time of the first reading of a feed() starting now whose t is past seconds. */
static uint32_t firstReadingAfter(float seconds)
{
    const uint32_t afterMs = (uint32_t)lroundf(seconds * 1000);
    return ms + INTERVAL_MS + (afterMs / INTERVAL_MS + 1) * INTERVAL_MS;
}

static int count(ShotEvent event)
{
    int n = 0;
    for (const Event &e : events)
    {
        n += e.event == event ? 1 : 0;
    }
    return n;
}

static float flat(float) { return 0; }

/** When the first drop falls in shot(). */
static const float FIRST_DROP_S = 7;

/** Flat, then the first drop; the flow begins at 0.5 g/s and speeds up by 0.125 g/s², 3 g/s at 20 s. */
static float shot(float t)
{
    const float tau = t - FIRST_DROP_S;
    return tau > 0 ? 0.5f * tau + 0.0625f * tau * tau : 0;
}

/** The flow of shot() at t, in g/s. */
static float shotFlow(float t)
{
    const float tau = t - FIRST_DROP_S;
    return tau > 0 ? 0.5f + 0.125f * tau : 0;
}

/** When the pump stops in stop(). */
static const float PUMP_STOP_S = 30;

/** shot() until the pump stops at stopS, then its flow falls linearly to 0 within fadeS, then flat. */
static float stoppedAt(float t, float stopS, float fadeS = 1)
{
    if (t <= stopS)
    {
        return shot(t);
    }
    const float s = std::min(t - stopS, fadeS);
    return shot(stopS) + shotFlow(stopS) * (s - s * s / (2 * fadeS));
}

static float stop(float t) { return stoppedAt(t, PUMP_STOP_S); }

/** Beans: a handful of 2 g per reading for 3 readings, a pause of 1 s, until 17 g. */
static float handfuls(float t)
{
    const int reading = (int)lroundf(t * 1000 / INTERVAL_MS);
    const int cycle = 3 + 1000 / INTERVAL_MS;
    const int steps = 3 * (reading / cycle) + std::min(reading % cycle + 1, 3);
    return std::min(2.0f * steps, 17.0f);
}

/** The cup tared and steady for 3 s, then shot(). Returns the true first drop. */
static uint32_t tareAndShoot(uint32_t shotMs = 25000, float noiseSigma = 0)
{
    tare();
    feed(3000, flat, noiseSigma);
    const uint32_t firstDrop = firstReadingAfter(FIRST_DROP_S);
    feed(shotMs, shot, noiseSigma);
    return firstDrop;
}

void test_a_smooth_rise_is_accepted_after_the_watch(void)
{
    const uint32_t firstDrop = tareAndShoot();
    TEST_ASSERT_EQUAL(1, (int)events.size());
    TEST_ASSERT_TRUE(events[0].event == ShotEvent::Accepted);
    TEST_ASSERT_UINT32_WITHIN(750, firstDrop + 10750, events[0].ms);
    TEST_ASSERT_UINT32_WITHIN(200, firstDrop, detector->firstDropMs());
    TEST_ASSERT_EQUAL(detector->firstDropMs(), detector->rise().front().ms);
    TEST_ASSERT_EQUAL(events[0].ms, detector->rise().back().ms);
    TEST_ASSERT_TRUE(detector->isShotRunning());
}

// e.g. the cup set down, not tared
void test_nothing_before_a_steady_stretch_at_zero(void)
{
    tareGrams = CUP_GRAMS - 20;
    feed(3000, flat);
    feed(25000, shot);
    TEST_ASSERT_EQUAL(0, (int)events.size());
    TEST_ASSERT_FALSE(detector->isShotRunning());
}

// only net changes with a tare, the detector watches grams
void test_a_tare_during_the_steady_stretch_changes_nothing(void)
{
    tareGrams = CUP_GRAMS - 2;
    feed(1500, flat);
    tare();
    feed(1500, flat);
    const uint32_t firstDrop = firstReadingAfter(FIRST_DROP_S);
    feed(25000, shot);
    TEST_ASSERT_EQUAL(1, count(ShotEvent::Accepted));
    TEST_ASSERT_UINT32_WITHIN(200, firstDrop, detector->firstDropMs());
}

// e.g. sliding the scale under the portafilter
void test_a_bump_before_the_drops_arms_again_and_the_shot_is_found(void)
{
    tare();
    // armed 2 s into the rest, so the bump comes once it is
    feed(2500, flat);
    feed(400, [](float) { return 3.0f; });
    feed(5000 - 400, flat);
    const uint32_t firstDrop = firstReadingAfter(FIRST_DROP_S);
    feed(25000, shot);
    TEST_ASSERT_EQUAL(1, (int)events.size());
    TEST_ASSERT_TRUE(events[0].event == ShotEvent::Accepted);
    TEST_ASSERT_UINT32_WITHIN(200, firstDrop, detector->firstDropMs());
}

// a drift of 0.003 g/s is steady and never 0.1 g above the baseline that follows it, but it is above 1σ of a scale
// without noise, which is 0: the creep fills the history before the shot
void test_a_long_creep_before_the_shot_does_not_hide_it(void)
{
    tare();
    feed(3000, flat);
    const float creepS = 30;
    const float creepGrams = 0.09f;
    feed((uint32_t)(creepS * 1000), [=](float t) { return creepGrams * t / creepS; });
    TEST_ASSERT_EQUAL(0, (int)events.size());
    TEST_ASSERT_EQUAL(ShotDetector::HISTORY_READINGS, (int)detector->rise().size());

    // the shot begins right on the creep, at 1.5 g/s, too fast to arm again on it
    const uint32_t creepEndMs = ms;
    feed(25000, [=](float t) { return creepGrams + 1.5f * t + 0.0625f * t * t; });
    TEST_ASSERT_EQUAL(1, count(ShotEvent::Accepted));
    TEST_ASSERT_UINT32_WITHIN(400, creepEndMs, detector->firstDropMs());
}

void test_handfuls_are_rejected_and_wait_for_a_tare(void)
{
    tare();
    feed(3000, flat);
    feed(12000, handfuls);
    TEST_ASSERT_EQUAL(0, (int)events.size());

    // a shot into the beans, without a tare
    feed(30000, [](float t) { return 17 + shot(t); });
    TEST_ASSERT_EQUAL(0, (int)events.size());
}

// like Beans 2, 23 g in 10 s: over the floor and under the ceiling all along, only the bursts show
void test_handfuls_that_keep_coming_are_rejected(void)
{
    tare();
    feed(3000, flat);
    feed(15000, [](float t) { return 2.0f * ((int)lroundf(t * 1000 / INTERVAL_MS) / 11 + 1); });
    TEST_ASSERT_EQUAL(0, (int)events.size());
}

// e.g. a few drops that stop
void test_a_rise_that_stops_flowing_is_rejected(void)
{
    tare();
    feed(3000, flat);
    feed(15000, [](float t) { return 0.5f * std::min(t, 3.0f); });
    TEST_ASSERT_EQUAL(0, (int)events.size());
}

void test_a_pause_right_at_the_start_of_the_watch_is_not_rejected(void)
{
    // the flow over 2 s reaches the floor 0.71 s after the first drop
    const float pause = FIRST_DROP_S + 0.71f + 0.3f;
    tare();
    feed(3000, flat);
    feed(25000, [pause](float t) { return t < pause ? shot(t) : t < pause + 0.4f ? shot(pause) : shot(t - 0.4f); });
    TEST_ASSERT_EQUAL(1, count(ShotEvent::Accepted));
}

/** A pour that reaches gramsPerSecond within 1 s, smoothly, and stays there. */
static void pour(float gramsPerSecond)
{
    tare();
    feed(3000, flat);
    feed(15000, [gramsPerSecond](float t)
         { return t < 1 ? gramsPerSecond / 2 * t * t : gramsPerSecond / 2 + gramsPerSecond * (t - 1); });
}

void test_a_steady_pour_over_the_ceiling_is_rejected(void)
{
    pour(6);
    TEST_ASSERT_EQUAL(0, (int)events.size());

    // as smooth, below the ceiling: it was the ceiling, not the smoothness
    tearDown();
    setUp();
    pour(4);
    TEST_ASSERT_EQUAL(1, count(ShotEvent::Accepted));
}

// e.g. a spoon set into the cup: for a moment it looks like a flow, but nothing flows
void test_a_step_that_stays_flat_is_rejected(void)
{
    tare();
    feed(3000, flat);
    feed(15000, [](float t) { return t > 0 ? 1.0f : 0.0f; });
    TEST_ASSERT_EQUAL(0, (int)events.size());
}

// like the 5 kg load cell
void test_noisy_scale(void)
{
    const uint32_t firstDrop = tareAndShoot(25000, 0.1f);
    TEST_ASSERT_EQUAL(1, (int)events.size());
    TEST_ASSERT_TRUE(events[0].event == ShotEvent::Accepted);
    TEST_ASSERT_UINT32_WITHIN(1000, firstDrop, detector->firstDropMs());

    tearDown();
    setUp();
    tare();
    feed(3000, flat, 0.1f);
    feed(12000, handfuls, 0.1f);
    TEST_ASSERT_EQUAL(0, (int)events.size());
}

// slow first drops on a noisy scale tilt the steady stretch before the rise is noticed: it stays armed
void test_a_noisy_shot_tilting_the_stretch_is_found(void)
{
    // with this seed the stretch's flow passes STEADY_FLOW before the averaged weight passes the rise threshold
    noise.seed(146);
    const uint32_t firstDrop = tareAndShoot(25000, 0.1f);
    TEST_ASSERT_EQUAL(1, count(ShotEvent::Accepted));
    TEST_ASSERT_UINT32_WITHIN(1000, firstDrop, detector->firstDropMs());
}

// e.g. a drifting scale, or drips below the floor: given up 10 s after it was noticed, and the shot right on top isn't tared
void test_a_rise_below_the_floor_is_given_up(void)
{
    tare();
    feed(3000, flat);
    feed(16000, [](float t) { return 0.1f * t; });
    feed(25000, [](float t) { return 1.6f + shot(FIRST_DROP_S + t); });
    TEST_ASSERT_EQUAL(0, (int)events.size());
}

// a slow start: its flow over 2 s reaches the floor at 2.7 s, and a reading 0.1 g low right after pulls it under for a moment
void test_a_dip_under_the_floor_right_at_the_start_of_the_watch_is_not_rejected(void)
{
    tare();
    feed(3000, flat);
    const uint32_t firstDrop = firstReadingAfter(0);
    feed(25000,
         [](float t)
         {
             const float grams = t > 0 ? 0.1f * t + 0.015f * t * t : 0;
             return fabsf(t - 2.79f) < 0.045f ? grams - 0.1f : grams;
         });
    TEST_ASSERT_EQUAL(1, count(ShotEvent::Accepted));
    // rejected there, it would arm again and be accepted with a first drop seconds late
    TEST_ASSERT_UINT32_WITHIN(1000, firstDrop, detector->firstDropMs());
}

void test_reset_and_gap_go_back_to_waiting(void)
{
    const float middle = FIRST_DROP_S + 5;
    tareAndShoot((uint32_t)(middle * 1000));
    detector->reset();
    feed(15000, [middle](float t) { return shot(middle + t); });
    TEST_ASSERT_EQUAL(0, (int)events.size());

    tareAndShoot((uint32_t)(middle * 1000));
    // the next reading 600 ms after the last
    ms += 600 - INTERVAL_MS;
    feed(15000, [middle](float t) { return shot(middle + t); });
    TEST_ASSERT_EQUAL(0, (int)events.size());

    tare();
    feed(2000, flat);
    feed(25000, shot);
    TEST_ASSERT_EQUAL(1, count(ShotEvent::Accepted));
}

void test_the_pump_stop_is_reported_once(void)
{
    tare();
    feed(3000, flat);
    const uint32_t pumpStop = firstReadingAfter(PUMP_STOP_S);
    // up to the last reading before the stop, a multiple of 90 ms
    const float before = 29.97f;
    feed((uint32_t)lroundf(before * 1000), stop);
    TEST_ASSERT_EQUAL(1, (int)events.size());
    TEST_ASSERT_TRUE(events[0].event == ShotEvent::Accepted);
    TEST_ASSERT_TRUE(detector->isShotRunning());

    feed(10000, [before](float t) { return stop(before + t); });
    TEST_ASSERT_EQUAL(2, (int)events.size());
    TEST_ASSERT_TRUE(events[1].event == ShotEvent::PumpStopped);
    // without noise it lands within a reading of the stop
    TEST_ASSERT_UINT32_WITHIN(200, pumpStop, detector->pumpStopMs());
    TEST_ASSERT_FALSE(detector->isShotRunning());
}

// e.g. a shot that chokes: its flow slows to 60 % within 2 s and keeps flowing
void test_a_slowing_flow_is_no_pump_stop(void)
{
    tare();
    feed(3000, flat);
    feed(40000,
         [](float t)
         {
             if (t <= PUMP_STOP_S)
             {
                 return shot(t);
             }
             // from the flow at the stop down to 60 % of it over 2 s
             const float flow = shotFlow(PUMP_STOP_S);
             const float s = std::min(t - PUMP_STOP_S, 2.0f);
             return shot(PUMP_STOP_S) + flow * (s - 0.1f * s * s) + 0.6f * flow * (t - PUMP_STOP_S - s);
         });
    TEST_ASSERT_EQUAL(1, count(ShotEvent::Accepted));
    TEST_ASSERT_EQUAL(0, count(ShotEvent::PumpStopped));
    TEST_ASSERT_TRUE(detector->isShotRunning());
}

// a click ends that shot
void test_a_pump_stop_while_the_cup_is_off_is_not_reported(void)
{
    tare();
    feed(3000, flat);
    feed(40000,
         [](float t)
         {
             const float grams = stoppedAt(t, 26);
             return t >= 25 && t < 28 ? grams - 150 : grams;
         });
    TEST_ASSERT_EQUAL(1, count(ShotEvent::Accepted));
    TEST_ASSERT_EQUAL(0, count(ShotEvent::PumpStopped));
    TEST_ASSERT_TRUE(detector->isShotRunning());
}

// the pump stopped while the cup was off; drips below the floor that stop afterwards are no pump stop either
void test_drips_that_stop_are_no_pump_stop(void)
{
    tare();
    feed(3000, flat);
    // 3 g of drips, more than after a real shot, so they rise past CUP_BACK_RISE_GRAMS and only the floor is left
    feed(65000,
         [](float t)
         {
             const float grams = stoppedAt(t, 26) + 0.1f * std::min(std::max(t - 29, 0.0f), 30.0f);
             return t >= 25 && t < 28 ? grams - 150 : grams;
         });
    TEST_ASSERT_EQUAL(1, count(ShotEvent::Accepted));
    TEST_ASSERT_EQUAL(0, count(ShotEvent::PumpStopped));
}

// on the 5 kg load cell the 1 s flows of a flat cup scatter by about 0.1 g/s: noise alone could look like a pump stop
void test_a_noisy_flat_cup_after_a_pump_stop_while_it_was_off_is_no_pump_stop(void)
{
    tare();
    feed(3000, flat, 0.1f);
    feed(48000,
         [](float t)
         {
             const float grams = stoppedAt(t, 26);
             return t >= 25 && t < 28 ? grams - 150 : grams;
         },
         0.1f);
    TEST_ASSERT_EQUAL(1, count(ShotEvent::Accepted));
    TEST_ASSERT_EQUAL(0, count(ShotEvent::PumpStopped));
    TEST_ASSERT_TRUE(detector->isShotRunning());
}

// the first reading of a lift only partly down pulls the last flow under half; the next one shows the lift
void test_a_lift_that_starts_partly_down_is_no_pump_stop(void)
{
    tare();
    feed(3000, flat);
    feed(40000,
         [](float t)
         {
             const float grams = shot(t);
             return t >= 22 && t < 22.09f ? grams - 4.9f : t >= 22.09f && t < 23 ? grams - 150 : grams;
         });
    TEST_ASSERT_EQUAL(1, count(ShotEvent::Accepted));
    TEST_ASSERT_EQUAL(0, count(ShotEvent::PumpStopped));
    TEST_ASSERT_TRUE(detector->isShotRunning());
}

void test_reset_and_gap_end_a_running_shot(void)
{
    const float accepted = 25;
    tareAndShoot((uint32_t)(accepted * 1000));
    TEST_ASSERT_TRUE(detector->isShotRunning());
    detector->reset();
    TEST_ASSERT_FALSE(detector->isShotRunning());
    feed(15000, [accepted](float t) { return stop(accepted + t); });
    TEST_ASSERT_EQUAL(1, (int)events.size());

    tareAndShoot((uint32_t)(accepted * 1000));
    TEST_ASSERT_EQUAL(2, count(ShotEvent::Accepted));
    TEST_ASSERT_TRUE(detector->isShotRunning());
    // the next reading 600 ms after the last
    ms += 600 - INTERVAL_MS;
    feed(15000, [accepted](float t) { return stop(accepted + 0.6f + t); });
    TEST_ASSERT_FALSE(detector->isShotRunning());
    TEST_ASSERT_EQUAL(0, count(ShotEvent::PumpStopped));
}

void test_the_pump_stop_after_the_cup_was_lifted_is_found(void)
{
    tare();
    feed(3000, flat);
    const uint32_t pumpStop = firstReadingAfter(PUMP_STOP_S);
    // the coffee goes on flowing into the lifted cup
    feed(40000,
         [](float t)
         {
             const float grams = stop(t);
             return t >= 20 && t < 21 ? grams - 150 : grams;
         });
    TEST_ASSERT_EQUAL(1, count(ShotEvent::Accepted));
    TEST_ASSERT_EQUAL(1, count(ShotEvent::PumpStopped));
    TEST_ASSERT_UINT32_WITHIN(500, pumpStop, detector->pumpStopMs());
}

void test_after_the_shot_it_waits_for_a_tare(void)
{
    tare();
    feed(3000, flat);
    // the pump stop is noticed about 1 s after it, at about 31 s
    feed(32000, stop);
    TEST_ASSERT_EQUAL(2, (int)events.size());
    TEST_ASSERT_TRUE(events[1].event == ShotEvent::PumpStopped);

    // a second shot into the same cup right away, not tared: its first drop 3 s later
    const float first = stop(40);
    feed(40000, [first](float t) { return first + stop(t + FIRST_DROP_S - 3); });
    TEST_ASSERT_EQUAL(2, (int)events.size());
    TEST_ASSERT_FALSE(detector->isShotRunning());

    const float both = 2 * first;
    tare(both);
    feed(3000, [both](float) { return both; });
    feed(25000, [both](float t) { return both + shot(t); });
    TEST_ASSERT_EQUAL(3, (int)events.size());
    TEST_ASSERT_TRUE(events[2].event == ShotEvent::Accepted);
}

// like the 5 kg load cell
void test_noisy_pump_stop(void)
{
    tare();
    feed(3000, flat, 0.1f);
    const uint32_t at29 = firstReadingAfter(PUMP_STOP_S - 1);
    const uint32_t pumpStop = firstReadingAfter(PUMP_STOP_S);
    feed(40000, stop, 0.1f);
    TEST_ASSERT_EQUAL(1, count(ShotEvent::Accepted));
    TEST_ASSERT_EQUAL(1, count(ShotEvent::PumpStopped));
    TEST_ASSERT_TRUE(events.back().event == ShotEvent::PumpStopped);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(at29, events.back().ms);
    TEST_ASSERT_UINT32_WITHIN(1000, pumpStop, detector->pumpStopMs());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_a_smooth_rise_is_accepted_after_the_watch);
    RUN_TEST(test_nothing_before_a_steady_stretch_at_zero);
    RUN_TEST(test_a_tare_during_the_steady_stretch_changes_nothing);
    RUN_TEST(test_a_bump_before_the_drops_arms_again_and_the_shot_is_found);
    RUN_TEST(test_a_long_creep_before_the_shot_does_not_hide_it);
    RUN_TEST(test_handfuls_are_rejected_and_wait_for_a_tare);
    RUN_TEST(test_handfuls_that_keep_coming_are_rejected);
    RUN_TEST(test_a_rise_that_stops_flowing_is_rejected);
    RUN_TEST(test_a_pause_right_at_the_start_of_the_watch_is_not_rejected);
    RUN_TEST(test_a_steady_pour_over_the_ceiling_is_rejected);
    RUN_TEST(test_a_step_that_stays_flat_is_rejected);
    RUN_TEST(test_noisy_scale);
    RUN_TEST(test_a_noisy_shot_tilting_the_stretch_is_found);
    RUN_TEST(test_a_rise_below_the_floor_is_given_up);
    RUN_TEST(test_a_dip_under_the_floor_right_at_the_start_of_the_watch_is_not_rejected);
    RUN_TEST(test_reset_and_gap_go_back_to_waiting);
    RUN_TEST(test_the_pump_stop_is_reported_once);
    RUN_TEST(test_a_slowing_flow_is_no_pump_stop);
    RUN_TEST(test_a_pump_stop_while_the_cup_is_off_is_not_reported);
    RUN_TEST(test_drips_that_stop_are_no_pump_stop);
    RUN_TEST(test_a_noisy_flat_cup_after_a_pump_stop_while_it_was_off_is_no_pump_stop);
    RUN_TEST(test_a_lift_that_starts_partly_down_is_no_pump_stop);
    RUN_TEST(test_reset_and_gap_end_a_running_shot);
    RUN_TEST(test_the_pump_stop_after_the_cup_was_lifted_is_found);
    RUN_TEST(test_after_the_shot_it_waits_for_a_tare);
    RUN_TEST(test_noisy_pump_stop);
    UNITY_END();
}
