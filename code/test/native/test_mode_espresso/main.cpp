#include "mock/mock_interface.h"
#include "mock/mock_display.h"
#include "mock/mock_canvas.h"
#include "mocks.h"
#include "modes/mode_espresso.h"
#include "stopwatch.h"
#include "millis.h"
#include "mock/mock_sample_source.h"
#include "weighing/weight_pipeline.h"
#include <unity.h>

static MockWeightSource *weightSource;
static Stopwatch *stopwatch;
static RecordingCanvas *canvas;

ModeEspresso *modeEspresso;

void setUp(void)
{
    Display::reset();
    Interface::reset();
    weightSource = new MockWeightSource();
    stopwatch = new Stopwatch();
    canvas = new RecordingCanvas();
    modeEspresso = new ModeEspresso(*weightSource, *stopwatch);
}

void tearDown(void)
{
    delete modeEspresso;
    delete canvas;
    delete stopwatch;
    delete weightSource;
}

static void renderFrame()
{
    canvas->clear();
    modeEspresso->render(*canvas);
}

void test_encoder_adjusts_target_weight(void)
{
    modeEspresso->update();
    int32_t targetWeight = modeEspresso->getTargetWeightMg();

    // turn encoder right increases weighht by 0.1g per tick
    Interface::encoderTicks = 5;
    modeEspresso->update();
    TEST_ASSERT_EQUAL(targetWeight + 500, modeEspresso->getTargetWeightMg());

    // turn encoder left decreases weight by 0.1g per tick
    Interface::encoderTicks = -10;
    modeEspresso->update();
    TEST_ASSERT_EQUAL(targetWeight - 500, modeEspresso->getTargetWeightMg());
}

void test_encoder_clamps_weight_between_min_and_max(void)
{
    // turn encoder left
    Interface::encoderTicks = -100000;
    modeEspresso->update();
    TEST_ASSERT_EQUAL(MIN_TARGET_WEIGHT_MG, modeEspresso->getTargetWeightMg());

    // turn encoder right
    Interface::encoderTicks = 1000000;
    modeEspresso->update();
    TEST_ASSERT_EQUAL(MAX_TARGET_WEIGHT_MG, modeEspresso->getTargetWeightMg());
}

void test_measures_while_the_stopwatch_runs(void)
{
    TEST_ASSERT_FALSE(modeEspresso->isMeasuring());
    Interface::encoderClick = ClickType::SINGLE;
    modeEspresso->update();
    TEST_ASSERT_TRUE(modeEspresso->isMeasuring());
    Interface::encoderClick = ClickType::SINGLE;
    modeEspresso->update();
    TEST_ASSERT_FALSE(modeEspresso->isMeasuring());
}

void test_clicking_encoder_starts_stopwatch(void)
{
    // initially time on watch is 0
    modeEspresso->update();
    renderFrame();
    TEST_ASSERT_TRUE(canvas->hasText("0.0s"));

    // click encoder to start stopwatch
    Interface::encoderClick = ClickType::SINGLE;
    modeEspresso->update();
    TEST_ASSERT_TRUE(stopwatch->isRunning());

    // click again to stop stopwatch
    Interface::encoderClick = ClickType::SINGLE;
    modeEspresso->update();
    TEST_ASSERT_FALSE(stopwatch->isRunning());
}

void test_render_shows_current_and_target_weight(void)
{
    weightSource->weight = 10;
    modeEspresso->update();
    renderFrame();
    TEST_ASSERT_TRUE(canvas->hasText("10.0g/36.0g"));
}

// right at zero, the noise is either side of it
void test_render_shows_no_minus_on_zero(void)
{
    weightSource->weight = -0.04f;
    modeEspresso->update();
    renderFrame();
    TEST_ASSERT_TRUE(canvas->hasText("0.0g/36.0g"));
}

void test_render_shows_only_elapsed_time_without_estimate(void)
{
    modeEspresso->update();
    TEST_ASSERT_TRUE(modeEspresso->isWaitingForEstimate());

    renderFrame();
    for (const RecordingCanvas::Text &t : canvas->texts)
    {
        TEST_ASSERT_NULL(strchr(t.text.c_str(), '|'));
    }
}

void test_update_does_not_draw(void)
{
    weightSource->weight = 10;
    modeEspresso->update();
    TEST_ASSERT_EQUAL(0, Display::mockCanvas.clears);
    TEST_ASSERT_EQUAL(0, Display::mockCanvas.flushes);
    TEST_ASSERT_EQUAL(0, (int)Display::mockCanvas.texts.size());
}

static Reading readingAt(unsigned long ms, float grams) { return {static_cast<uint32_t>(ms), 0, grams, grams, grams, 0}; }

// Readings that waited in the queue arrive together, each must count at its own time.
void test_estimate_uses_the_time_of_each_reading(void)
{
    const unsigned long start = now();
    Interface::encoderClick = ClickType::SINGLE;
    modeEspresso->update();
    Interface::reset();

    sleep_for(1500);
    // 10 g/s: 36 g after 3.6 s
    weightSource->readings = {readingAt(start + 500, 5), readingAt(start + 1000, 10), readingAt(start + 1500, 15)};
    modeEspresso->update();

    TEST_ASSERT_FALSE(modeEspresso->isWaitingForEstimate());
    TEST_ASSERT_INT_WITHIN(5, 2100, modeEspresso->getRemainingTimeMs());
}

void test_estimate_skips_readings_from_before_the_start(void)
{
    const unsigned long start = now();
    Interface::encoderClick = ClickType::SINGLE;
    modeEspresso->update();
    Interface::reset();

    sleep_for(1500);
    weightSource->readings = {readingAt(start - 500, 100), readingAt(start + 500, 5), readingAt(start + 1000, 10),
                              readingAt(start + 1500, 15)};
    modeEspresso->update();

    TEST_ASSERT_INT_WITHIN(5, 2100, modeEspresso->getRemainingTimeMs());
}

// The click that starts the shot tares, after the readings of that loop were
// made with the old tare. A cup on the scale must not count as espresso.
void test_readings_from_before_the_tare_are_skipped(void)
{
    QueuedSamples samples;
    WeightPipeline pipeline(samples);
    ModeEspresso espresso(pipeline, *stopwatch);
    pipeline.setGramsPerCount(0.01f);

    // a 150 g cup, measured in the same millisecond as the click
    samples.samples = {{static_cast<uint32_t>(now()), 15000}};
    pipeline.update();
    Interface::encoderClick = ClickType::SINGLE;
    espresso.update();
    Interface::reset();

    TEST_ASSERT_TRUE(stopwatch->isRunning());
}

// a detected shot: first drop 2.4 s ago, the rise readings since then, 10 g/s
void test_start_shot_backdates_time_and_estimate(void)
{
    const unsigned long t = now();
    sleep_for(2400);
    modeEspresso->startShot(t, ShotStartSource::FirstDrop, t,
                            {readingAt(t + 400, 4), readingAt(t + 1400, 14), readingAt(t + 2400, 24)});
    TEST_ASSERT_TRUE(modeEspresso->isMeasuring());
    TEST_ASSERT_UINT32_WITHIN(1, 2400, stopwatch->getTime());
    TEST_ASSERT_FALSE(modeEspresso->isWaitingForEstimate());
    // 36 g at 3.6 s
    TEST_ASSERT_INT_WITHIN(5, 1200, modeEspresso->getRemainingTimeMs());
    TEST_ASSERT_TRUE(ShotStartSource::FirstDrop == modeEspresso->getStartSource());
    TEST_ASSERT_EQUAL(t, modeEspresso->getFirstDropMs());
}

void test_start_shot_does_not_tare(void)
{
    weightSource->weight = 5;
    modeEspresso->startShot(now(), ShotStartSource::FirstDrop, now(), {});
    TEST_ASSERT_EQUAL_FLOAT(5, weightSource->getWeight());
}

// a detected shot is accepted: the pump stop ends it, not the target weight
void test_start_shot_over_the_target_keeps_running(void)
{
    const unsigned long t = now();
    sleep_for(1000);
    modeEspresso->startShot(t, ShotStartSource::FirstDrop, t, {readingAt(t + 1000, 40)});
    TEST_ASSERT_TRUE(modeEspresso->isAccepted());
    TEST_ASSERT_TRUE(modeEspresso->isMeasuring());
}

void test_target_weight_stops_a_shot_that_is_not_accepted(void)
{
    const unsigned long start = now();
    Interface::encoderClick = ClickType::SINGLE;
    modeEspresso->update();
    Interface::reset();
    TEST_ASSERT_FALSE(modeEspresso->isAccepted());

    sleep_for(2000);
    weightSource->readings = {readingAt(start + 2000, 36)};
    modeEspresso->update();
    TEST_ASSERT_FALSE(stopwatch->isRunning());
}

void test_target_weight_does_not_stop_an_accepted_shot(void)
{
    const unsigned long t = now();
    sleep_for(1000);
    modeEspresso->startShot(t, ShotStartSource::FirstDrop, t, {readingAt(t + 1000, 10)});
    sleep_for(2000);
    weightSource->weight = 40;
    weightSource->readings = {readingAt(t + 2000, 36), readingAt(t + 3000, 40)};
    modeEspresso->update();
    TEST_ASSERT_TRUE(stopwatch->isRunning());

    renderFrame();
    TEST_ASSERT_TRUE(canvas->hasText("40.0g/36.0g"));
    // the progress bar is full
    TEST_ASSERT_GREATER_THAN(0, canvas->boxes);
}

void test_set_accepted_on_a_click_started_shot(void)
{
    const unsigned long start = now();
    Interface::encoderClick = ClickType::SINGLE;
    modeEspresso->update();
    Interface::reset();
    TEST_ASSERT_FALSE(modeEspresso->isAccepted());

    modeEspresso->setAccepted(true);
    TEST_ASSERT_TRUE(modeEspresso->isAccepted());
    sleep_for(2000);
    weightSource->readings = {readingAt(start + 2000, 40)};
    modeEspresso->update();
    TEST_ASSERT_TRUE(stopwatch->isRunning());
}

void test_click_start_clears_accepted(void)
{
    modeEspresso->setAccepted(true);
    Interface::encoderClick = ClickType::SINGLE;
    modeEspresso->update();
    TEST_ASSERT_TRUE(stopwatch->isRunning());
    TEST_ASSERT_FALSE(modeEspresso->isAccepted());
}

void test_stop_shot_at_backdates_the_time(void)
{
    set_now(20000);
    modeEspresso->startShot(1000, ShotStartSource::FirstDrop, 1000, {});
    modeEspresso->stopShotAt(18000);
    TEST_ASSERT_EQUAL(17000, stopwatch->getTime());
    TEST_ASSERT_FALSE(modeEspresso->isMeasuring());
}

// the loop that confirms a shot passes its readings to startShot(), then update() gets them again
void test_readings_already_fed_are_skipped(void)
{
    const unsigned long t = now();
    sleep_for(1500);
    std::vector<Reading> rise = {readingAt(t + 500, 5), readingAt(t + 1000, 10), readingAt(t + 1500, 15)};
    modeEspresso->startShot(t, ShotStartSource::FirstDrop, t, rise);
    const int32_t remaining = modeEspresso->getRemainingTimeMs();
    // a later reading on the same line, after the same ones again: only the new one may count
    weightSource->readings = rise;
    weightSource->readings.push_back(readingAt(t + 1500, 30));
    modeEspresso->update();
    TEST_ASSERT_EQUAL(remaining, modeEspresso->getRemainingTimeMs());
}

void test_click_start_has_source_click(void)
{
    Interface::encoderClick = ClickType::SINGLE;
    modeEspresso->update();
    TEST_ASSERT_TRUE(ShotStartSource::Click == modeEspresso->getStartSource());
    TEST_ASSERT_EQUAL(0, modeEspresso->getFirstDropMs());
}

// e.g. a detected shot that wasn't one: going back must not leave the timer running
void test_leaving_stops_the_shot(void)
{
    Interface::encoderClick = ClickType::SINGLE;
    modeEspresso->update();
    TEST_ASSERT_TRUE(modeEspresso->isMeasuring());
    modeEspresso->leave();
    TEST_ASSERT_FALSE(modeEspresso->isMeasuring());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_encoder_adjusts_target_weight);
    RUN_TEST(test_measures_while_the_stopwatch_runs);
    RUN_TEST(test_clicking_encoder_starts_stopwatch);
    RUN_TEST(test_encoder_clamps_weight_between_min_and_max);
    RUN_TEST(test_render_shows_current_and_target_weight);
    RUN_TEST(test_render_shows_no_minus_on_zero);
    RUN_TEST(test_render_shows_only_elapsed_time_without_estimate);
    RUN_TEST(test_update_does_not_draw);
    RUN_TEST(test_estimate_uses_the_time_of_each_reading);
    RUN_TEST(test_estimate_skips_readings_from_before_the_start);
    RUN_TEST(test_readings_from_before_the_tare_are_skipped);
    RUN_TEST(test_start_shot_backdates_time_and_estimate);
    RUN_TEST(test_start_shot_does_not_tare);
    RUN_TEST(test_start_shot_over_the_target_keeps_running);
    RUN_TEST(test_target_weight_stops_a_shot_that_is_not_accepted);
    RUN_TEST(test_target_weight_does_not_stop_an_accepted_shot);
    RUN_TEST(test_set_accepted_on_a_click_started_shot);
    RUN_TEST(test_click_start_clears_accepted);
    RUN_TEST(test_stop_shot_at_backdates_the_time);
    RUN_TEST(test_readings_already_fed_are_skipped);
    RUN_TEST(test_click_start_has_source_click);
    RUN_TEST(test_leaving_stops_the_shot);
    UNITY_END();
}
