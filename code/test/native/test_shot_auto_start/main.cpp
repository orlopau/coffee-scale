#include <unity.h>

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

static MockWeightSource *weights;
static Stopwatch *stopwatch;
static MockMode *scale;
static MockMode *recipes;
static ModeEspresso *espresso;
static Mode *modes[3];
static ModeManager *manager;
static ShotAutoStart *autoStart;
/** When the last shot()'s drops began. */
static unsigned long firstDrop;

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
    autoStart->setAutoSwitch(true);
    autoStart->setOffsetMs(0);
    autoStart->setTypicalShotSeconds(30);
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

/** One pass of the main loop, 100 ms after the last, with one reading. */
static void loop(float grams, float net, float flow = 0)
{
    sleep_for(100);
    weights->readings = {{static_cast<uint32_t>(now()), 0, grams, net, net, flow}};
    weights->weight = net;
    autoStart->update();
    manager->update();
}

/** A tared cup, then drops flowing in at gramsPerSecond for 3 s. */
static void shot(float gramsPerSecond = 1.2f)
{
    for (int i = 0; i < 5; i++)
    {
        loop(100, 0);
    }
    for (int i = 0; i < 30; i++)
    {
        const float added = 0.4f + gramsPerSecond * 0.1f * i;
        loop(100 + added, added, gramsPerSecond);
        if (i == 0)
        {
            firstDrop = now();
        }
    }
}

void test_switches_from_the_scale_mode_and_starts(void)
{
    shot();
    TEST_ASSERT_EQUAL_PTR(espresso, manager->activeMode());
    TEST_ASSERT_TRUE(espresso->isMeasuring());
    TEST_ASSERT_UINT32_WITHIN(150, now() - firstDrop, stopwatch->getTime());
    TEST_ASSERT_EQUAL(firstDrop, espresso->getFirstDropMs());
}

void test_starts_on_the_espresso_screen(void)
{
    manager->switchTo(espresso);
    shot();
    TEST_ASSERT_TRUE(espresso->isMeasuring());
    TEST_ASSERT_TRUE(ShotStartSource::FirstDrop == espresso->getStartSource());
}

void test_starts_on_the_espresso_screen_with_switching_off(void)
{
    autoStart->setAutoSwitch(false);
    manager->switchTo(espresso);
    shot();
    TEST_ASSERT_TRUE(espresso->isMeasuring());
}

void test_stays_in_the_scale_mode_with_switching_off(void)
{
    autoStart->setAutoSwitch(false);
    shot();
    TEST_ASSERT_EQUAL_PTR(scale, manager->activeMode());
    TEST_ASSERT_FALSE(espresso->isMeasuring());
}

void test_recipe_mode_is_not_interrupted(void)
{
    manager->switchTo(recipes);
    shot();
    TEST_ASSERT_EQUAL_PTR(recipes, manager->activeMode());
    TEST_ASSERT_FALSE(espresso->isMeasuring());
}

void test_nothing_while_the_switcher_is_open(void)
{
    Interface::encoderClick = ClickType::LONG;
    manager->update();
    Interface::encoderClick = ClickType::NONE;
    shot();
    TEST_ASSERT_NULL(manager->activeMode());
    TEST_ASSERT_FALSE(espresso->isMeasuring());
}

// e.g. the scale mode's stopwatch timing a pour-over by hand
void test_nothing_while_measuring(void)
{
    scale->measuring = true;
    shot();
    TEST_ASSERT_EQUAL_PTR(scale, manager->activeMode());
    TEST_ASSERT_FALSE(espresso->isMeasuring());
}

void test_a_running_shot_is_not_restarted(void)
{
    manager->switchTo(espresso);
    Interface::encoderClick = ClickType::SINGLE;
    loop(100, 0);
    // the mock keeps a click until reset
    Interface::encoderClick = ClickType::NONE;
    TEST_ASSERT_TRUE(espresso->isMeasuring());
    shot();
    TEST_ASSERT_TRUE(espresso->isMeasuring());
    TEST_ASSERT_TRUE(ShotStartSource::Click == espresso->getStartSource());
}

void test_offset_moves_the_start_back(void)
{
    autoStart->setOffsetMs(3000);
    shot();
    TEST_ASSERT_UINT32_WITHIN(150, now() - firstDrop + 3000, stopwatch->getTime());
    TEST_ASSERT_EQUAL(firstDrop, espresso->getFirstDropMs());
}

void test_offset_longer_than_uptime_starts_at_zero(void)
{
    set_now(2000);
    autoStart->setOffsetMs(10000);
    shot();
    TEST_ASSERT_TRUE(espresso->isMeasuring());
    TEST_ASSERT_EQUAL(now(), stopwatch->getTime());
}

void test_expected_flow_follows_the_target_weight(void)
{
    // 6 g/s is too fast for 36 g in 30 s
    shot(6);
    TEST_ASSERT_EQUAL_PTR(scale, manager->activeMode());

    // 100 g in 30 s, 0.1 g per tick
    manager->switchTo(espresso);
    Interface::encoderTicks = 640;
    loop(100, 0);
    TEST_ASSERT_EQUAL(100000, espresso->getTargetWeightMg());
    manager->switchTo(scale);
    shot(6);
    TEST_ASSERT_EQUAL_PTR(espresso, manager->activeMode());
}

void test_typical_shot_time_sets_the_flow(void)
{
    // a turbo, a little below the most 36 g in 15 s may flow
    const float flow = 2.4f * ShotDetector::MAX_FLOW_FACTOR * 0.8f;
    shot(flow);
    TEST_ASSERT_EQUAL_PTR(scale, manager->activeMode());

    autoStart->setTypicalShotSeconds(15);
    shot(flow);
    TEST_ASSERT_EQUAL_PTR(espresso, manager->activeMode());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_switches_from_the_scale_mode_and_starts);
    RUN_TEST(test_starts_on_the_espresso_screen);
    RUN_TEST(test_starts_on_the_espresso_screen_with_switching_off);
    RUN_TEST(test_stays_in_the_scale_mode_with_switching_off);
    RUN_TEST(test_recipe_mode_is_not_interrupted);
    RUN_TEST(test_nothing_while_the_switcher_is_open);
    RUN_TEST(test_nothing_while_measuring);
    RUN_TEST(test_a_running_shot_is_not_restarted);
    RUN_TEST(test_offset_moves_the_start_back);
    RUN_TEST(test_offset_longer_than_uptime_starts_at_zero);
    RUN_TEST(test_expected_flow_follows_the_target_weight);
    RUN_TEST(test_typical_shot_time_sets_the_flow);
    UNITY_END();
}
