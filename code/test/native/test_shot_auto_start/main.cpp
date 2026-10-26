#include <unity.h>

#include <algorithm>
#include <math.h>

#include "detectors/shot_auto_start.h"
#include "millis.h"
#include "mocks.h"
#include "mock/mock_display.h"
#include "mock/mock_interface.h"
#include "mode_manager.h"
#include "modes/mode_espresso.h"
#include "stopwatch.h"

class MockMode : public Mode
{
public:
    explicit MockMode(const char *name) : name(name) {}
    void update() override {}
    void render(Canvas &canvas) override {}
    bool canSwitchMode() override { return true; }
    bool isMeasuring() override { return measuring; }
    const char *getName() override { return name; }
    const char *name;
    bool measuring = false;
};

/** The cup on the scale, tared before every test. */
static const float CUP_GRAMS = 100;
/** The scale's readings come this far apart. */
static const uint32_t INTERVAL_MS = 90;
/** 36 g, the default, down to 10 g. */
static const int TICKS_TO_10_G = -260;

static MockWeightSource *weights;
static Stopwatch *stopwatch;
static MockMode *scale;
static MockMode *recipes;
static ModeEspresso *espresso;
static Mode *modes[3];
static ModeManager *manager;
static ShotAutoStart *autoStart;
/** What the scale weighed at the last tare. */
static float tareGrams;
/** Time of the last reading. */
static uint32_t ms;
/** How far into the shot the readings of shotFeed() are, in ms. */
static uint32_t shotElapsedMs;
/** Time of the last reading before the shot's t = 0. */
static uint32_t shotOriginMs;

void setUp(void)
{
    Interface::reset();
    Display::reset();
    set_now(100000);
    weights = new MockWeightSource();
    stopwatch = new Stopwatch();
    scale = new MockMode("Scale");
    recipes = new MockMode("Recipes");
    espresso = new ModeEspresso(*weights, *stopwatch);
    modes[0] = scale;
    modes[1] = recipes;
    modes[2] = espresso;
    manager = new ModeManager(modes, 3, Display::mockCanvas);
    manager->begin();
    autoStart = new ShotAutoStart(*weights, *manager, *scale, *espresso);
    autoStart->setEnabled(true);
    autoStart->setOffsetMs(0);
    ms = now();
    shotElapsedMs = 0;
    tareGrams = CUP_GRAMS;
}

void tearDown(void)
{
    delete autoStart;
    delete manager;
    delete espresso;
    delete recipes;
    delete scale;
    delete stopwatch;
    delete weights;
}

/** When the pump stops in stop(), seconds into the shot. */
static const float PUMP_STOP_S = 30;
/** When the first drop falls in stop(), the pump started FIRST_DROP_S - offset before. */
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

/** shot() until the pump stops at PUMP_STOP_S, then its flow falls linearly to 0 within a second, then flat. */
static float stop(float t)
{
    if (t <= PUMP_STOP_S)
    {
        return shot(t);
    }
    const float s = std::min(t - PUMP_STOP_S, 1.0f);
    return shot(PUMP_STOP_S) + shotFlow(PUMP_STOP_S) * (s - s * s / 2);
}

/**
 * One pass of the main loop, one reading INTERVAL_MS after the last: the cup
 * plus grams on the scale.
 */
static void loop(float grams)
{
    sleep_for(INTERVAL_MS);
    ms = now();
    const float total = CUP_GRAMS + grams;
    weights->weight = total - tareGrams;
    weights->readings = {{ms, 0, total, total - tareGrams, total - tareGrams, 0}};
    autoStart->update();
    manager->update();
}

/** Readings for durationMs, grams the cup holds at t seconds from the first of them. */
template <typename Profile> static void feed(uint32_t durationMs, Profile gramsAt)
{
    for (uint32_t t = 0; t < durationMs; t += INTERVAL_MS)
    {
        loop(gramsAt(t / 1000.0f));
    }
}

/** The cup tared and steady. */
static void steady(uint32_t durationMs)
{
    tareGrams = CUP_GRAMS;
    feed(durationMs, [](float) { return 0.0f; });
}

/**
 * The next durationMs of stop(), the shot goes on from where the last shotFeed()
 * stopped. The first one begins it, its t = 0 is one reading from now.
 */
static void shotFeed(uint32_t durationMs)
{
    if (shotElapsedMs == 0)
    {
        shotOriginMs = ms;
    }
    const uint32_t from = shotElapsedMs;
    feed(durationMs, [from](float t) { return stop(from / 1000.0f + t); });
    shotElapsedMs += (durationMs + INTERVAL_MS - 1) / INTERVAL_MS * INTERVAL_MS;
}

/** When the first drop falls in the shot, the first reading past FIRST_DROP_S. */
static uint32_t firstDrop() { return shotOriginMs + INTERVAL_MS + (uint32_t)(FIRST_DROP_S * 1000 / INTERVAL_MS + 1) * INTERVAL_MS; }

/** When the pump stops in the shot. */
static uint32_t pumpStop() { return shotOriginMs + INTERVAL_MS + (uint32_t)(PUMP_STOP_S * 1000); }

/** The cup tared and steady, then the shot up to 25 s into it, accepted at about 18 s. */
static void steadyAndShoot()
{
    steady(3000);
    shotFeed(25000);
}

/** A click on the espresso screen in one loop, with a reading that goes with it. */
static void click(float grams)
{
    Interface::encoderClick = ClickType::SINGLE;
    loop(grams);
    // the mock keeps a click until reset
    Interface::encoderClick = ClickType::NONE;
}

void test_off_does_nothing(void)
{
    autoStart->setEnabled(false);
    steadyAndShoot();
    TEST_ASSERT_EQUAL_PTR(scale, manager->activeMode());
    TEST_ASSERT_FALSE(espresso->isMeasuring());

    manager->switchTo(espresso);
    shotElapsedMs = 0;
    steadyAndShoot();
    TEST_ASSERT_EQUAL_PTR(espresso, manager->activeMode());
    TEST_ASSERT_FALSE(espresso->isMeasuring());
}

void test_switches_from_the_scale_mode(void)
{
    autoStart->setOffsetMs(7000);
    steadyAndShoot();
    TEST_ASSERT_EQUAL_PTR(espresso, manager->activeMode());
    TEST_ASSERT_TRUE(espresso->isMeasuring());
    TEST_ASSERT_TRUE(ShotStartSource::FirstDrop == espresso->getStartSource());
    TEST_ASSERT_UINT32_WITHIN(200, firstDrop(), espresso->getFirstDropMs());
    // the start is the first drop minus the offset
    TEST_ASSERT_UINT32_WITHIN(200, now() - firstDrop() + 7000, stopwatch->getTime());
}

void test_starts_on_the_espresso_screen(void)
{
    manager->switchTo(espresso);
    autoStart->setOffsetMs(3000);
    steadyAndShoot();
    TEST_ASSERT_EQUAL_PTR(espresso, manager->activeMode());
    TEST_ASSERT_TRUE(espresso->isMeasuring());
    TEST_ASSERT_TRUE(ShotStartSource::FirstDrop == espresso->getStartSource());
    TEST_ASSERT_UINT32_WITHIN(200, now() - firstDrop() + 3000, stopwatch->getTime());
}

void test_accepts_a_click_started_shot(void)
{
    manager->switchTo(espresso);
    click(0);
    TEST_ASSERT_TRUE(espresso->isMeasuring());
    TEST_ASSERT_FALSE(espresso->isAccepted());

    steady(3000);
    shotFeed(25000);
    TEST_ASSERT_TRUE(espresso->isAccepted());
    TEST_ASSERT_TRUE(espresso->isMeasuring());
    TEST_ASSERT_TRUE(ShotStartSource::Click == espresso->getStartSource());

    // the shot is past 10 g by now, an accepted shot isn't stopped by that
    Interface::encoderTicks = TICKS_TO_10_G;
    shotFeed(2000);
    TEST_ASSERT_EQUAL(10000, espresso->getTargetWeightMg());
    TEST_ASSERT_TRUE(espresso->isMeasuring());

    // its pump stop does
    shotFeed(8000);
    TEST_ASSERT_FALSE(espresso->isMeasuring());
}

void test_stops_an_accepted_shot_at_the_pump_stop(void)
{
    autoStart->setOffsetMs(7000);
    steady(3000);
    shotFeed(28000);
    TEST_ASSERT_TRUE(espresso->isMeasuring());
    shotFeed(7000);
    TEST_ASSERT_FALSE(espresso->isMeasuring());
    TEST_ASSERT_FALSE(stopwatch->isRunning());
    const unsigned long start = firstDrop() - 7000;
    TEST_ASSERT_UINT32_WITHIN(500, pumpStop() - start, stopwatch->getTime());

    // the time stays
    const unsigned long stopped = stopwatch->getTime();
    sleep_for(5000);
    TEST_ASSERT_EQUAL(stopped, stopwatch->getTime());
}

void test_ignores_recipes_and_the_switcher(void)
{
    manager->switchTo(recipes);
    steadyAndShoot();
    TEST_ASSERT_EQUAL_PTR(recipes, manager->activeMode());
    TEST_ASSERT_FALSE(espresso->isMeasuring());

    manager->switchTo(scale);
    Interface::encoderClick = ClickType::LONG;
    manager->update();
    Interface::encoderClick = ClickType::NONE;
    shotElapsedMs = 0;
    steadyAndShoot();
    TEST_ASSERT_NULL(manager->activeMode());
    TEST_ASSERT_FALSE(espresso->isMeasuring());
}

// e.g. the scale mode's stopwatch timing a pour-over by hand
void test_nothing_while_the_scale_mode_measures(void)
{
    scale->measuring = true;
    steadyAndShoot();
    TEST_ASSERT_EQUAL_PTR(scale, manager->activeMode());
    TEST_ASSERT_FALSE(espresso->isMeasuring());
}

void test_offset_longer_than_uptime_starts_at_zero(void)
{
    set_now(500);
    autoStart->setOffsetMs(10000);
    // the first drop falls 3.5 s into the shot's feed, 5 s after the first reading
    steady(1500);
    feed(15000, [](float t) { return shot(t + FIRST_DROP_S - 3.5f); });
    TEST_ASSERT_TRUE(espresso->isMeasuring());
    TEST_ASSERT_EQUAL(now(), stopwatch->getTime());
}

void test_after_a_click_ends_the_shot_it_waits_for_a_tare(void)
{
    steady(3000);
    shotFeed(20000);
    TEST_ASSERT_EQUAL_PTR(espresso, manager->activeMode());
    TEST_ASSERT_TRUE(espresso->isMeasuring());

    // the click doesn't tare when it stops
    Interface::encoderClick = ClickType::SINGLE;
    shotFeed(90);
    Interface::encoderClick = ClickType::NONE;
    TEST_ASSERT_FALSE(espresso->isMeasuring());

    // the pump goes on and stops, then a second pour on top of the cup
    shotFeed(20000);
    TEST_ASSERT_FALSE(espresso->isMeasuring());
    const float topped = stop(1000);
    feed(3000, [topped](float) { return topped; });
    feed(25000, [topped](float t) { return topped + shot(t); });
    TEST_ASSERT_FALSE(espresso->isMeasuring());
}

void test_a_shot_the_detector_lost_stops_at_the_target_weight(void)
{
    steadyAndShoot();
    TEST_ASSERT_TRUE(espresso->isAccepted());
    TEST_ASSERT_TRUE(espresso->isMeasuring());

    // readings 600 ms apart reset the detector
    sleep_for(600);
    shotFeed(2 * INTERVAL_MS);
    TEST_ASSERT_FALSE(espresso->isAccepted());
    TEST_ASSERT_TRUE(espresso->isMeasuring());

    // the weight is over 10 g, the target weight stops the shot again
    Interface::encoderTicks = TICKS_TO_10_G;
    shotFeed(2 * INTERVAL_MS);
    TEST_ASSERT_FALSE(espresso->isMeasuring());
}

/** The rest of the pump's shot, 25 s on: no shot starts, a second at a time. */
static void assertNoShotWhilePumpRunsOn()
{
    for (int i = 0; i < 25; i++)
    {
        shotFeed(1000);
        TEST_ASSERT_FALSE(espresso->isMeasuring());
        TEST_ASSERT_FALSE(stopwatch->isRunning());
    }
}

/** A click started shot on the espresso screen, the pump's first drop 7 s into the shot feed. */
static void clickStartAndPump(uint32_t shotMs, int ticks = 0)
{
    manager->switchTo(espresso);
    Interface::encoderTicks = ticks;
    click(0);
    TEST_ASSERT_TRUE(espresso->isMeasuring());
    steady(3000);
    shotFeed(shotMs);
}

// the detector accepts a shot at about 18 s, long after the shot ended
void test_a_shot_the_target_weight_stopped_before_it_was_accepted_is_not_restarted(void)
{
    // 5 g target
    clickStartAndPump(14000, -310);
    // 5 g are in the cup at about 12 s
    TEST_ASSERT_EQUAL(5000, espresso->getTargetWeightMg());
    TEST_ASSERT_FALSE(espresso->isMeasuring());
    assertNoShotWhilePumpRunsOn();
    TEST_ASSERT_EQUAL_PTR(espresso, manager->activeMode());
}

void test_a_shot_a_click_stopped_before_it_was_accepted_is_not_restarted(void)
{
    clickStartAndPump(12000);
    TEST_ASSERT_TRUE(espresso->isMeasuring());
    Interface::encoderClick = ClickType::SINGLE;
    shotFeed(INTERVAL_MS);
    Interface::encoderClick = ClickType::NONE;
    TEST_ASSERT_FALSE(espresso->isMeasuring());
    assertNoShotWhilePumpRunsOn();
}

void test_leaving_to_the_scale_mode_before_the_shot_was_accepted_stays_there(void)
{
    clickStartAndPump(12000);
    TEST_ASSERT_TRUE(espresso->isMeasuring());
    manager->switchTo(scale);
    TEST_ASSERT_FALSE(espresso->isMeasuring());
    assertNoShotWhilePumpRunsOn();
    TEST_ASSERT_EQUAL_PTR(scale, manager->activeMode());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_off_does_nothing);
    RUN_TEST(test_switches_from_the_scale_mode);
    RUN_TEST(test_starts_on_the_espresso_screen);
    RUN_TEST(test_accepts_a_click_started_shot);
    RUN_TEST(test_stops_an_accepted_shot_at_the_pump_stop);
    RUN_TEST(test_ignores_recipes_and_the_switcher);
    RUN_TEST(test_nothing_while_the_scale_mode_measures);
    RUN_TEST(test_offset_longer_than_uptime_starts_at_zero);
    RUN_TEST(test_after_a_click_ends_the_shot_it_waits_for_a_tare);
    RUN_TEST(test_a_shot_the_detector_lost_stops_at_the_target_weight);
    RUN_TEST(test_a_shot_the_target_weight_stopped_before_it_was_accepted_is_not_restarted);
    RUN_TEST(test_a_shot_a_click_stopped_before_it_was_accepted_is_not_restarted);
    RUN_TEST(test_leaving_to_the_scale_mode_before_the_shot_was_accepted_stays_there);
    UNITY_END();
}
