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
    UNITY_END();
}
