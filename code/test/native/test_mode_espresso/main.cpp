#include "mock/mock_interface.h"
#include "mock/mock_display.h"
#include "mock/mock_canvas.h"
#include "mocks.h"
#include "modes/mode_espresso.h"
#include "stopwatch.h"
#include <unity.h>

static MockWeightSensor *weightSensor;
static Stopwatch *stopwatch;
static RecordingCanvas *canvas;

ModeEspresso *modeEspresso;

void setUp(void)
{
    Display::reset();
    Interface::reset();
    weightSensor = new MockWeightSensor();
    stopwatch = new Stopwatch();
    canvas = new RecordingCanvas();
    modeEspresso = new ModeEspresso(*weightSensor, *stopwatch);
}

void tearDown(void)
{
    delete modeEspresso;
    delete canvas;
    delete stopwatch;
    delete weightSensor;
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
    weightSensor->weight = 10;
    modeEspresso->update();
    renderFrame();
    TEST_ASSERT_TRUE(canvas->hasText("10.0g/36.0g"));
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
    weightSensor->weight = 10;
    modeEspresso->update();
    TEST_ASSERT_EQUAL(0, Display::mockCanvas.clears);
    TEST_ASSERT_EQUAL(0, Display::mockCanvas.flushes);
    TEST_ASSERT_EQUAL(0, (int)Display::mockCanvas.texts.size());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_encoder_adjusts_target_weight);
    RUN_TEST(test_clicking_encoder_starts_stopwatch);
    RUN_TEST(test_encoder_clamps_weight_between_min_and_max);
    RUN_TEST(test_render_shows_current_and_target_weight);
    RUN_TEST(test_render_shows_only_elapsed_time_without_estimate);
    RUN_TEST(test_update_does_not_draw);
    UNITY_END();
}
