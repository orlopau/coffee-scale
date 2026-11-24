#include <unity.h>

#include <string>

#include "grinder/grinder_link.h"
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
    const char *getName() override { return name; }
    bool isMeasuring() override { return measuring; }
    const char *name;
    bool measuring = false;
};

static MockWeightSource *weights;
static Stopwatch *stopwatch;
// the scale mode and another mode, e.g. the settings, are plain mock modes
static MockMode *scale;
static MockMode *other;
static ModeEspresso *espresso;
static Mode *modeList[3];
static ModeManager *modes;
static Xenia::Machine *machine;
static GrinderLink *link;

void setUp(void)
{
    Interface::reset();
    Display::reset();
    set_now(100000);
    weights = new MockWeightSource();
    stopwatch = new Stopwatch();
    scale = new MockMode("Scale");
    other = new MockMode("Other");
    espresso = new ModeEspresso(*weights, *stopwatch);
    modeList[0] = scale;
    modeList[1] = espresso;
    modeList[2] = other;
    modes = new ModeManager(modeList, 3, Display::mockCanvas);
    modes->begin();
    machine = new Xenia::Machine();
    link = new GrinderLink(*machine, *weights, *modes, *scale, *espresso);
    link->setEnabled(true);
}

void tearDown(void)
{
    delete link;
    delete machine;
    delete modes;
    delete espresso;
    delete other;
    delete scale;
    delete stopwatch;
    delete weights;
}

/** The knob press, and the grinder's poll that sees it. */
static void knob()
{
    machine->start(now());
    machine->mako(now());
}

static void loop()
{
    link->update();
    modes->update();
}

/** A reading at now(), through a loop. */
static void reading(float grams)
{
    weights->newReading(grams);
    loop();
}

/** The reply after the end of the shot, once the machine moved on. */
static std::string reply()
{
    machine->update(now());
    return machine->mako(now());
}

static bool has(const std::string &body, const char *part)
{
    return body.find(part) != std::string::npos;
}

/** A detected shot, started now: 40 g is the target, the detector stops it. */
static void startDetectedShot()
{
    espresso->startShot(now(), ShotStartSource::FirstDrop, now(), {});
    espresso->setAccepted(true);
}

void test_knob_press_opens_espresso_and_tares(void)
{
    machine->setBrewWeight(40);
    knob();
    loop();
    TEST_ASSERT_TRUE(modes->activeMode() == espresso);
    TEST_ASSERT_EQUAL(1, weights->tares);
    TEST_ASSERT_EQUAL(40000, espresso->getTargetWeightMg());
}

void test_no_brew_weight_keeps_the_target(void)
{
    espresso->setTargetWeightMg(36000);
    knob();
    loop();
    TEST_ASSERT_TRUE(modes->activeMode() == espresso);
    TEST_ASSERT_EQUAL(36000, espresso->getTargetWeightMg());
}

void test_a_knob_press_is_not_taken_twice(void)
{
    knob();
    loop();
    TEST_ASSERT_EQUAL(1, weights->tares);
    loop();
    loop();
    TEST_ASSERT_EQUAL(1, weights->tares);
}

void test_time_to_the_target_is_sent(void)
{
    machine->setBrewWeight(40);
    knob();
    loop();
    startDetectedShot();
    loop();
    sleep_for(25000);
    reading(40.0f);
    sleep_for(2000);
    reading(45.0f);
    espresso->stopShotAt(now());
    loop();

    const std::string body = reply();
    TEST_ASSERT_TRUE(has(body, "\"MA_STATUS\":4"));
    TEST_ASSERT_TRUE(has(body, "\"MA_EXTRACTION_STATUS\":1"));
    TEST_ASSERT_TRUE(has(body, "\"REAL_EXTRACTION_TIME_MS\":25000"));
    TEST_ASSERT_TRUE(has(body, "\"PU_SENS_FLOW_METER_VOLUME\":45.0"));
}

void test_late_stop_sends_the_same_time(void)
{
    machine->setBrewWeight(40);
    knob();
    loop();
    startDetectedShot();
    loop();
    sleep_for(25000);
    reading(40.0f);
    sleep_for(5000);
    reading(50.0f);
    espresso->stopShotAt(now());
    loop();

    const std::string body = reply();
    TEST_ASSERT_TRUE(has(body, "\"MA_EXTRACTION_STATUS\":1"));
    TEST_ASSERT_TRUE(has(body, "\"REAL_EXTRACTION_TIME_MS\":25000"));
}

void test_early_stop_is_aborted(void)
{
    machine->setBrewWeight(40);
    knob();
    loop();
    startDetectedShot();
    loop();
    sleep_for(20000);
    reading(32.0f);
    espresso->stopShotAt(now());
    loop();
    // the drips stay under the target
    for (int i = 0; i < 6; i++)
    {
        sleep_for(500);
        reading(32.4f);
    }
    TEST_ASSERT_TRUE(machine->phase() == Xenia::Phase::Running);
    sleep_for(500);
    reading(32.4f);

    TEST_ASSERT_TRUE(has(reply(), "\"MA_EXTRACTION_STATUS\":2"));
}

void test_drips_reach_the_target(void)
{
    machine->setBrewWeight(40);
    knob();
    loop();
    startDetectedShot();
    loop();
    sleep_for(27000);
    reading(39.5f);
    espresso->stopShotAt(now());
    loop();
    sleep_for(800);
    reading(40.1f);

    const std::string body = reply();
    TEST_ASSERT_TRUE(has(body, "\"MA_EXTRACTION_STATUS\":1"));
    TEST_ASSERT_TRUE(has(body, "\"REAL_EXTRACTION_TIME_MS\":27000"));
    TEST_ASSERT_TRUE(has(body, "\"PU_SENS_FLOW_METER_VOLUME\":40.1"));
}

void test_drips_before_a_backdated_stop_do_not_count_past_it(void)
{
    machine->setBrewWeight(40);
    knob();
    loop();
    const unsigned long start = now();
    startDetectedShot();
    loop();
    sleep_for(27000);
    reading(39.5f);
    sleep_for(600);
    // a drip crosses the target before the detector reports the pump stop, 1 s back
    reading(40.1f);
    espresso->stopShotAt(start + 27000);
    loop();

    const std::string body = reply();
    TEST_ASSERT_TRUE(has(body, "\"MA_EXTRACTION_STATUS\":1"));
    TEST_ASSERT_TRUE(has(body, "\"REAL_EXTRACTION_TIME_MS\":27000"));
}

void test_time_runs_from_the_click_without_detection(void)
{
    machine->setBrewWeight(40);
    knob();
    loop();
    // 4 s later the click starts the stopwatch, and tares again
    sleep_for(4000);
    Interface::encoderClick = ClickType::SINGLE;
    loop();
    Interface::reset();
    TEST_ASSERT_EQUAL(2, weights->tares);
    TEST_ASSERT_TRUE(espresso->isMeasuring());
    loop();

    sleep_for(19900);
    reading(30.0f);
    sleep_for(100);
    reading(40.0f);
    // the espresso screen stops at the target
    TEST_ASSERT_FALSE(espresso->isMeasuring());
    loop();

    const std::string body = reply();
    TEST_ASSERT_TRUE(has(body, "\"MA_EXTRACTION_STATUS\":1"));
    TEST_ASSERT_UINT32_WITHIN(100, 20000, espresso->targetReachedMs());
    TEST_ASSERT_TRUE(has(body, "\"REAL_EXTRACTION_TIME_MS\":20000"));
}

void test_knob_press_in_another_mode_is_aborted(void)
{
    modes->switchTo(other);
    knob();
    loop();
    TEST_ASSERT_TRUE(modes->activeMode() == other);
    TEST_ASSERT_EQUAL(0, weights->tares);
    TEST_ASSERT_TRUE(has(reply(), "\"MA_EXTRACTION_STATUS\":2"));
}

void test_knob_press_while_the_scale_mode_measures_is_aborted(void)
{
    scale->measuring = true;
    modes->switchTo(scale);
    knob();
    loop();
    TEST_ASSERT_TRUE(modes->activeMode() == scale);
    TEST_ASSERT_EQUAL(0, weights->tares);
    TEST_ASSERT_TRUE(has(reply(), "\"MA_EXTRACTION_STATUS\":2"));
}

void test_knob_press_during_a_running_shot_is_aborted(void)
{
    modes->switchTo(espresso);
    startDetectedShot();
    knob();
    loop();
    TEST_ASSERT_TRUE(espresso->isMeasuring());
    TEST_ASSERT_EQUAL(0, weights->tares);
    TEST_ASSERT_TRUE(has(reply(), "\"MA_EXTRACTION_STATUS\":2"));
}

void test_leaving_espresso_aborts(void)
{
    knob();
    loop();
    TEST_ASSERT_TRUE(modes->activeMode() == espresso);
    modes->switchTo(scale);
    loop();
    TEST_ASSERT_TRUE(has(reply(), "\"MA_EXTRACTION_STATUS\":2"));
}

void test_knob_press_leaves_the_finished_shot(void)
{
    modes->switchTo(espresso);
    startDetectedShot();
    sleep_for(6000);
    reading(30.0f);
    espresso->stopShotAt(now());
    TEST_ASSERT_TRUE(espresso->isShowingShot());

    knob();
    loop();
    TEST_ASSERT_FALSE(espresso->isShowingShot());
    TEST_ASSERT_EQUAL(1, weights->tares);
}

void test_a_huge_brew_weight_is_the_largest_target(void)
{
    // past what lroundf() can hold, on the ESP32 and natively
    machine->setBrewWeight(1e20f);
    knob();
    loop();
    TEST_ASSERT_EQUAL(MAX_TARGET_WEIGHT_MG, espresso->getTargetWeightMg());
}

void test_a_shot_that_started_runs_past_the_timeout(void)
{
    machine->setBrewWeight(40);
    knob();
    loop();
    // the shot starts shortly before the 3 minutes are up
    sleep_for(170000);
    startDetectedShot();
    loop();
    sleep_for(25000);
    reading(40.0f);
    espresso->stopShotAt(now());
    loop();

    const std::string body = reply();
    TEST_ASSERT_TRUE(has(body, "\"MA_EXTRACTION_STATUS\":1"));
    TEST_ASSERT_TRUE(has(body, "\"REAL_EXTRACTION_TIME_MS\":25000"));
}

void test_knob_press_while_finishing_starts_the_next_shot(void)
{
    // pressed on the wrong screen: aborted, and the grinder sees it finishing
    modes->switchTo(other);
    knob();
    loop();
    // the next loop moves the machine on, as on the scale
    loop();
    TEST_ASSERT_TRUE(machine->phase() == Xenia::Phase::Finishing);
    TEST_ASSERT_TRUE(machine->aborted());

    // sooner it would be a repeat of the first
    sleep_for(Xenia::START_REPEAT_MS);
    modes->switchTo(espresso);
    knob();
    loop();
    TEST_ASSERT_EQUAL(1, weights->tares);
    TEST_ASSERT_TRUE(machine->phase() == Xenia::Phase::Running);
    TEST_ASSERT_TRUE(has(machine->mako(now()), "\"MA_STATUS\":3"));
}

void test_the_grinders_repeat_of_an_aborted_knob_press_starts_nothing(void)
{
    modes->switchTo(other);
    knob();
    loop();
    loop();
    TEST_ASSERT_TRUE(machine->phase() == Xenia::Phase::Finishing);

    // meanwhile on the scale mode: the repeat must not switch to espresso and tare
    modes->switchTo(scale);
    loop();
    sleep_for(8700);
    knob();
    loop();
    sleep_for(8200);
    knob();
    loop();
    TEST_ASSERT_EQUAL(0, weights->tares);
    TEST_ASSERT_TRUE(modes->activeMode() == scale);
    TEST_ASSERT_TRUE(machine->phase() != Xenia::Phase::Running);
}

void test_disabled_does_nothing(void)
{
    link->setEnabled(false);
    knob();
    loop();
    TEST_ASSERT_TRUE(modes->activeMode() == scale);
    TEST_ASSERT_EQUAL(0, weights->tares);
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_knob_press_opens_espresso_and_tares);
    RUN_TEST(test_no_brew_weight_keeps_the_target);
    RUN_TEST(test_a_knob_press_is_not_taken_twice);
    RUN_TEST(test_time_to_the_target_is_sent);
    RUN_TEST(test_late_stop_sends_the_same_time);
    RUN_TEST(test_early_stop_is_aborted);
    RUN_TEST(test_drips_reach_the_target);
    RUN_TEST(test_drips_before_a_backdated_stop_do_not_count_past_it);
    RUN_TEST(test_time_runs_from_the_click_without_detection);
    RUN_TEST(test_knob_press_in_another_mode_is_aborted);
    RUN_TEST(test_knob_press_while_the_scale_mode_measures_is_aborted);
    RUN_TEST(test_knob_press_during_a_running_shot_is_aborted);
    RUN_TEST(test_leaving_espresso_aborts);
    RUN_TEST(test_knob_press_leaves_the_finished_shot);
    RUN_TEST(test_a_huge_brew_weight_is_the_largest_target);
    RUN_TEST(test_a_shot_that_started_runs_past_the_timeout);
    RUN_TEST(test_knob_press_while_finishing_starts_the_next_shot);
    RUN_TEST(test_the_grinders_repeat_of_an_aborted_knob_press_starts_nothing);
    RUN_TEST(test_disabled_does_nothing);
    return UNITY_END();
}
