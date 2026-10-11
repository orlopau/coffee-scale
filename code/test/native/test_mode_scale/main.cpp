#include <unity.h>
#include "modes/mode_scale.h"
#include "mocks.h"
#include "mock/mock_interface.h"
#include "mock/mock_display.h"
#include "stopwatch.h"

#include "millis.h"
#include "settings.h"

static Stopwatch *stopwatch;
static MockWeightSource *weightSource;

ModeScale *modeScale;

void setUp(void)
{
    Interface::reset();
    Display::reset();
    stopwatch = new Stopwatch();
    weightSource = new MockWeightSource();
    modeScale = new ModeScale(*weightSource, *stopwatch);
}

void tearDown(void)
{
    delete stopwatch;
    delete modeScale;
    delete weightSource;
}

void test_stopwatch_start_when_click(void)
{
    TEST_ASSERT_FALSE(stopwatch->isRunning());
    Interface::encoderClick = ClickType::SINGLE;
    modeScale->update();
    TEST_ASSERT_TRUE(stopwatch->isRunning());
}

void test_stopwatch_stop_when_click_again(void)
{
    Interface::encoderClick = ClickType::SINGLE;
    modeScale->update();
    modeScale->update();
    TEST_ASSERT_FALSE(stopwatch->isRunning());
}

void test_loadcell_tare_when_encoder_rotated(void)
{
    weightSource->weight = 1;

    Interface::encoderDirection = Interface::EncoderDirection::CW;
    modeScale->update();
    TEST_ASSERT_EQUAL(0, weightSource->getWeight());
}

void test_display_shows_weight(void)
{
    weightSource->weight = 1;

    modeScale->update();
    modeScale->render(Display::mockCanvas);
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("1.00g"));
}

void test_display_shows_time(void)
{
    stopwatch->start();
    sleep_for(200);
    modeScale->update();
    modeScale->render(Display::mockCanvas);
    TEST_ASSERT_EQUAL_STRING("00:00.2", Display::mockCanvas.texts.back().text.c_str());
}

// Readings that waited in the queue arrive together, auto-tare must see all of them.
void test_auto_tare_sees_every_reading(void)
{
    Settings::setFloat(Settings::floatSettings[Settings::AUTO_TARE_0], 36);
    Settings::setFloat(Settings::floatSettings[Settings::AUTO_TARE_TOLERANCE], 1);
    modeScale->enter();
    weightSource->weight = 136;

    // the scale is steady at 100 g, then a 36 g cup is put on
    for (int i = 0; i < 32; i++)
    {
        float grams = i < 16 ? 100 : 136;
        weightSource->readings.push_back({static_cast<uint32_t>(i * 100), 0, grams, grams, grams, 0});
    }
    modeScale->update();

    TEST_ASSERT_EQUAL_FLOAT(0, weightSource->getWeight());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_stopwatch_start_when_click);
    RUN_TEST(test_stopwatch_stop_when_click_again);

    RUN_TEST(test_loadcell_tare_when_encoder_rotated);
    RUN_TEST(test_display_shows_weight);
    RUN_TEST(test_display_shows_time);
    RUN_TEST(test_auto_tare_sees_every_reading);
    UNITY_END();
}