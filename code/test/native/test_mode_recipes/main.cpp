#include <unity.h>
#include "mocks.h"
#include "stopwatch.h"
#include "modes/mode_recipe.h"
#include "millis.h"

static Stopwatch *stopwatch;
static MockLoadCell *loadCell;
static MockButtons *buttons;
static MockDisplay *display;

const Recipe RECIPES[] = {
    {"name1", "desc1", 3000, (2 + 3) * 100, 2, {
                                                   {"step1", 200, 500, 300, true},
                                                   {"step2", 500, 0, 300, false},
                                               }},
    {"name2", "desc2", 2000, 2 * 100, 2, {
                                             {"step1", 100, 0, 2 * 60 * 1000, false},
                                             {"step2", 100, 0, 30 * 1000, false},
                                         }},
    {"name3", "desc3", 3000, 2 * 100, 2, {
                                             {"step1", 100, 0, 2 * 60 * 1000, false},
                                             {"step2", 100, 0, 30 * 1000, false},
                                         }},
};

ModeRecipes *modeRecipes;

void setUp(void)
{
    stopwatch = new Stopwatch();
    loadCell = new MockLoadCell();
    buttons = new MockButtons();
    display = new MockDisplay();
    modeRecipes = new ModeRecipes(*loadCell, *buttons, *display, RECIPES, 3);
}

void tearDown(void)
{
    delete stopwatch;
    delete loadCell;
    delete buttons;
    delete display;
    delete modeRecipes;
}

void test_steps_forward_and_back(void)
{
    TEST_ASSERT_EQUAL(ModeRecipes::State::RECIPE_SELECTION, modeRecipes->state);
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(ModeRecipes::State::RECIPE_SUMMARY, modeRecipes->state);
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(ModeRecipes::State::RECIPE_CONFIG, modeRecipes->state);
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(ModeRecipes::State::RECIPE_PREPARE, modeRecipes->state);
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(ModeRecipes::State::RECIPE_BREWING, modeRecipes->state);
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(ModeRecipes::State::RECIPE_BREWING, modeRecipes->state);
    buttons->encoderClick = ClickType::LONG;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(ModeRecipes::State::RECIPE_PREPARE, modeRecipes->state);
    buttons->encoderClick = ClickType::LONG;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(ModeRecipes::State::RECIPE_CONFIG, modeRecipes->state);
    buttons->encoderClick = ClickType::LONG;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(ModeRecipes::State::RECIPE_SUMMARY, modeRecipes->state);
    buttons->encoderClick = ClickType::LONG;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(ModeRecipes::State::RECIPE_SELECTION, modeRecipes->state);
    buttons->encoderClick = ClickType::LONG;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(ModeRecipes::State::RECIPE_SELECTION, modeRecipes->state);
}

void test_recipe_switcher(void)
{
    // initial recipe is number 0
    TEST_ASSERT_EQUAL(0, modeRecipes->recipeIndex);

    // turning encoder to left should remain at 0
    buttons->encoderDirection = EncoderDirection::CCW;
    modeRecipes->update();
    TEST_ASSERT_EQUAL(0, modeRecipes->recipeIndex);

    // turning encoder to right should change to 1
    buttons->encoderDirection = EncoderDirection::CW;
    modeRecipes->update();
    TEST_ASSERT_EQUAL(1, modeRecipes->recipeIndex);

    // turning encoder to right should change to 2
    buttons->encoderDirection = EncoderDirection::CW;
    modeRecipes->update();
    TEST_ASSERT_EQUAL(2, modeRecipes->recipeIndex);

    // turning encoder to right should remain at 2
    buttons->encoderDirection = EncoderDirection::CW;
    modeRecipes->update();
    TEST_ASSERT_EQUAL(2, modeRecipes->recipeIndex);

    // turning encoder to left should change to 1
    buttons->encoderDirection = EncoderDirection::CCW;
    modeRecipes->update();
    TEST_ASSERT_EQUAL(1, modeRecipes->recipeIndex);
}

void test_recipe_switcher_select()
{
    // initial recipe is number 0
    TEST_ASSERT_EQUAL(0, modeRecipes->recipeIndex);

    // select recipe by clicking
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();

    // summary should be displayed on display
    TEST_ASSERT_EQUAL_STRING("name1", display->recipeName);
    TEST_ASSERT_EQUAL_STRING("desc1", display->recipeDescription);
}

void test_recipe_configuration()
{
    modeRecipes->state = ModeRecipes::State::RECIPE_CONFIG;
    modeRecipes->update();

    // configuration for coffee weight should be display
    // weight can be adjusted using rotary encoder
    // the resulting water weight should be displayed as well

    // initial weight and water weight are displayed
    TEST_ASSERT_EQUAL(3000, display->weightConfigWeightMg);
    // 1000mg coffee -> 2 pours with 2 and 3 times coffee weight as water respectively
    TEST_ASSERT_EQUAL(15000, display->weightConfigWaterWeightMl);

    // encoder ticks adjust coffee weight
    // turning encoder left should decrease
    buttons->encoderTicks = -1;
    modeRecipes->update();
    TEST_ASSERT_EQUAL(2000, display->weightConfigWeightMg);
    TEST_ASSERT_EQUAL(10000, display->weightConfigWaterWeightMl);

    // cant reduce to 0 or below
    buttons->encoderTicks = -3;
    modeRecipes->update();
    TEST_ASSERT_EQUAL(1000, display->weightConfigWeightMg);
    TEST_ASSERT_EQUAL(5000, display->weightConfigWaterWeightMl);
}

void test_recipe_brewing()
{
    // recipe prepare step, prompt to insert ground beans
    modeRecipes->state = ModeRecipes::State::RECIPE_PREPARE;
    modeRecipes->update();

    // next via click
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();
    buttons->encoderClick = ClickType::NONE;

    // scale should be tared
    // TODO: verify tare

    // brewing recipe 1
    // 2 pours with 2 and 3 times coffee weight as water respectively
    // should show weight to add and time to add it
    TEST_ASSERT_EQUAL(6000, display->recipeWeightToPourMg);
    TEST_ASSERT_EQUAL(500, display->recipeTimeToFinishMs);

    // 500ms pour time
    // after 200ms still 6000mg to pour and ~300ms left
    // still pour time not pause time
    sleep_for(201);
    modeRecipes->update();
    TEST_ASSERT_LESS_THAN(300, display->recipeTimeToFinishMs);
    TEST_ASSERT_GREATER_THAN(200, display->recipeTimeToFinishMs);
    TEST_ASSERT_FALSE(display->recipeIsPause);

    // wait another ~300ms
    sleep_for(301);
    modeRecipes->update();
    // now pause has started, 300ms pause time
    TEST_ASSERT_TRUE(display->recipeIsPause);
    TEST_ASSERT_LESS_THAN(300, display->recipeTimeToFinishMs);
    TEST_ASSERT_GREATER_THAN(200, display->recipeTimeToFinishMs);

    // wait another ~300ms
    sleep_for(301);
    modeRecipes->update();
    // now time should be 0
    TEST_ASSERT_EQUAL(0, display->recipeTimeToFinishMs);

    // next step reached by single click
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();
    buttons->encoderClick = ClickType::NONE;
    modeRecipes->update();

    // verify that second step has started
    // as loadcell always returns 0, total missing weight is now the 
    // weight of the first pour + the weight of the second pour = 6000 + 9000 = 15000
    TEST_ASSERT_EQUAL(15000, display->recipeWeightToPourMg);

    // as second pour only contains pause time, should instantly show pause time
    TEST_ASSERT_TRUE(display->recipeIsPause);
    TEST_ASSERT_EQUAL(300, display->recipeTimeToFinishMs);

    // after 300ms, recipe should be done
    sleep_for(301);
    modeRecipes->update();
    TEST_ASSERT_EQUAL(0, display->recipeTimeToFinishMs);

    // clicking button should advance to next step
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();
    buttons->encoderClick = ClickType::NONE;

    modeRecipes->update();
    TEST_ASSERT_EQUAL(ModeRecipes::RECIPE_DONE, modeRecipes->state);

    // clicking again should return to recipe selection
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();
    buttons->encoderClick = ClickType::NONE;

    modeRecipes->update();
    TEST_ASSERT_EQUAL(ModeRecipes::RECIPE_SELECTION, modeRecipes->state);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_steps_forward_and_back);
    RUN_TEST(test_recipe_switcher);
    RUN_TEST(test_recipe_switcher_select);
    RUN_TEST(test_recipe_configuration);
    RUN_TEST(test_recipe_brewing);
    UNITY_END();
}