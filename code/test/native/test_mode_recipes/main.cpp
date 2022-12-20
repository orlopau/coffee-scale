#include <unity.h>
#include "mocks.h"
#include "stopwatch.h"
#include "modes/mode_recipe.h"
#include "millis.h"

static Stopwatch *stopwatch;
static MockWeightSensor *weightSensor;
static MockButtons *buttons;
static MockDisplay *display;

const Recipe RECIPES[] = {
    {"name1", "desc1", 3000, (2 + 3) * RECIPE_RATIO_MUL, 2, static_cast<uint8_t>(AdjustableParameter::COFFEE_WEIGHT) | static_cast<uint8_t>(AdjustableParameter::RATIO), {
                                                                                                                                                                             {"step1", 2 * RECIPE_RATIO_MUL, 500, 300, true},
                                                                                                                                                                             {"step2", 5 * RECIPE_RATIO_MUL, 0, 300, false},
                                                                                                                                                                         }},
    {"name2", "desc2", 2000, 2 * RECIPE_RATIO_MUL, 2, 0, {
                                                             {"step1", RECIPE_RATIO_MUL, 0, 2 * 60 * 1000, false},
                                                             {"step2", RECIPE_RATIO_MUL, 0, 30 * 1000, false},
                                                         }},
    {"name3", "desc3", 3000, 2 * RECIPE_RATIO_MUL, 2, 0, {
                                                             {"step1", RECIPE_RATIO_MUL, 0, 2 * 60 * 1000, false},
                                                             {"step2", RECIPE_RATIO_MUL, 0, 30 * 1000, false},
                                                         }},
};

ModeRecipes *modeRecipes;

void setUp(void)
{
    stopwatch = new Stopwatch();
    weightSensor = new MockWeightSensor();
    buttons = new MockButtons();
    display = new MockDisplay();
    modeRecipes = new ModeRecipes(*weightSensor, *buttons, *display, RECIPES, 3);
}

void tearDown(void)
{
    delete stopwatch;
    delete weightSensor;
    delete buttons;
    delete display;
    delete modeRecipes;
}

void test_steps_forward_and_back(void)
{
    TEST_ASSERT_EQUAL(0, modeRecipes->getCurrentStepIndex());
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(1, modeRecipes->getCurrentStepIndex());
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(2, modeRecipes->getCurrentStepIndex());
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(3, modeRecipes->getCurrentStepIndex());
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(4, modeRecipes->getCurrentStepIndex());
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(5, modeRecipes->getCurrentStepIndex());
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();

    // Should not go further, as timer did not expire yet

    TEST_ASSERT_EQUAL(5, modeRecipes->getCurrentStepIndex());
    buttons->encoderClick = ClickType::LONG;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(4, modeRecipes->getCurrentStepIndex());
    buttons->encoderClick = ClickType::LONG;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(3, modeRecipes->getCurrentStepIndex());
    buttons->encoderClick = ClickType::LONG;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(2, modeRecipes->getCurrentStepIndex());
    buttons->encoderClick = ClickType::LONG;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(1, modeRecipes->getCurrentStepIndex());
    buttons->encoderClick = ClickType::LONG;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(0, modeRecipes->getCurrentStepIndex());
    buttons->encoderClick = ClickType::LONG;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(0, modeRecipes->getCurrentStepIndex());
}

void test_recipe_switcher(void)
{
    modeRecipes->update();

    // initial recipe is number 0
    TEST_ASSERT_EQUAL(0, display->switcherIndex);

    // turning encoder to left should remain at 0
    buttons->encoderDirection = EncoderDirection::CCW;
    modeRecipes->update();
    TEST_ASSERT_EQUAL(0, display->switcherIndex);

    // turning encoder to right should change to 1
    buttons->encoderDirection = EncoderDirection::CW;
    modeRecipes->update();
    TEST_ASSERT_EQUAL(1, display->switcherIndex);

    // turning encoder to right should change to 2
    buttons->encoderDirection = EncoderDirection::CW;
    modeRecipes->update();
    TEST_ASSERT_EQUAL(2, display->switcherIndex);

    // turning encoder to right should remain at 2
    buttons->encoderDirection = EncoderDirection::CW;
    modeRecipes->update();
    TEST_ASSERT_EQUAL(2, display->switcherIndex);

    // turning encoder to left should change to 1
    buttons->encoderDirection = EncoderDirection::CCW;
    modeRecipes->update();
    TEST_ASSERT_EQUAL(1, display->switcherIndex);
}

void test_recipe_switcher_select()
{
    modeRecipes->update();

    // initial recipe is number 0
    TEST_ASSERT_EQUAL(0, display->switcherIndex);

    // select recipe by clicking
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();

    // summary should be displayed on display
    TEST_ASSERT_EQUAL_STRING("name1", display->recipeName);
    TEST_ASSERT_EQUAL_STRING("desc1", display->recipeDescription);
}

void test_recipe_configuration_ratio()
{
    // select recipe
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();
    // summary
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();
    // config
    buttons->encoderClick = ClickType::NONE;

    TEST_ASSERT_EQUAL(2, modeRecipes->getCurrentStepIndex());
    modeRecipes->update();

    // as per adjustables of recipe 1, ratio and coffee weight can be adjusted
    // first the ratio is adjusted
    // initial ratio is as given in the recipe, i.e. 1 gram of coffee to 5 grams water
    // ratios must be divided by 100 to get the actual ratio
    TEST_ASSERT_EQUAL(1, display->ratioCoffee);
    TEST_ASSERT_EQUAL(5, display->ratioWater);

    // by turning encoder, ratio can be adjusted in steps of 0.1
    buttons->encoderTicks = 1;
    modeRecipes->update();
    TEST_ASSERT_EQUAL(5.1, display->ratioWater);
    buttons->encoderTicks = -1;
    modeRecipes->update();
    TEST_ASSERT_EQUAL(4.9, display->ratioWater);

    // ratio can not be reduced below 1:1
    buttons->encoderTicks = -10000;
    modeRecipes->update();
    TEST_ASSERT_EQUAL(1, display->ratioWater);

    // maxium ratio is limited by 64
    buttons->encoderTicks = 10000;
    modeRecipes->update();
    TEST_ASSERT_EQUAL(64 + 5, display->ratioWater);

    // return to normal ratio
    buttons->encoderTicks = 0;
    modeRecipes->update();
    TEST_ASSERT_EQUAL(5, display->ratioWater);
}

void test_recipe_configuration_weight()
{
    // select recipe
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();
    // summary
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();
    // config
    buttons->encoderClick = ClickType::NONE;

    TEST_ASSERT_EQUAL(2, modeRecipes->getCurrentStepIndex());
    modeRecipes->update();

    // skip adjusting ratio
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();
    buttons->encoderClick = ClickType::NONE;

    // now the coffee weight is adjusted
    // initial coffee weight is 3 grams
    TEST_ASSERT_EQUAL(3000, display->weightConfigWeightMg);
    // 3000mg coffee -> 2 pours with 2 and 3 times coffee weight as water respectively, resulting in 15g water
    TEST_ASSERT_EQUAL(15, display->weightConfigWaterWeightMl);

    // encoder ticks adjust coffee weight
    // turning encoder left should decrease
    buttons->encoderTicks = -1;
    modeRecipes->update();
    TEST_ASSERT_EQUAL(2000, display->weightConfigWeightMg);
    TEST_ASSERT_EQUAL(10, display->weightConfigWaterWeightMl);

    // cant reduce to 0 or below
    buttons->encoderTicks = -3;
    modeRecipes->update();
    TEST_ASSERT_EQUAL(1000, display->weightConfigWeightMg);
    TEST_ASSERT_EQUAL(5, display->weightConfigWaterWeightMl);

    // next click advances to recipe preparation
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();
    buttons->encoderClick = ClickType::NONE;
    TEST_ASSERT_EQUAL(4, modeRecipes->getCurrentStepIndex());
}

void test_recipe_brewing()
{
    // select recipe
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();
    // summary
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();
    // config ratio
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();
    // config weight
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();
    // prepare
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();
    buttons->encoderClick = ClickType::NONE;

    // recipe prepare step, prompt to insert ground beans
    TEST_ASSERT_EQUAL(5, modeRecipes->getCurrentStepIndex());
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
    TEST_ASSERT_EQUAL(6, modeRecipes->getCurrentStepIndex());

    // clicking again should return to recipe selection
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();
    buttons->encoderClick = ClickType::NONE;

    modeRecipes->update();
    TEST_ASSERT_EQUAL(0, modeRecipes->getCurrentStepIndex());
}

void test_adjusting_global_ratio_affects_pour_ratio()
{
    // select recipe
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();
    // summary
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();
    // config
    buttons->encoderClick = ClickType::NONE;

    // increase ratio by 5, doubling the ratio to 10
    buttons->encoderTicks = 50;
    modeRecipes->update();
    TEST_ASSERT_EQUAL(10, display->ratioWater);

    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();
    // weight config
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();
    // prepare
    buttons->encoderClick = ClickType::SINGLE;
    modeRecipes->update();
    // brewing

    // verify brewing step
    TEST_ASSERT_EQUAL(5, modeRecipes->getCurrentStepIndex());
    modeRecipes->update();

    // first pour had a ratio of 2, now it should be 4
    // coffee weight is 3000mg, so water weight should be 12000mg
    TEST_ASSERT_EQUAL(12000, display->recipeWeightToPourMg);
}

void test_recipe_insert_coffee_values()
{
    // for loop 4 times
    for (int i = 0; i < 4; i++)
    {
        buttons->encoderClick = ClickType::SINGLE;
        modeRecipes->update();
    }
    buttons->encoderClick = ClickType::NONE;
    TEST_ASSERT_EQUAL(4, modeRecipes->getCurrentStepIndex());
    modeRecipes->update();

    // should show required weight and current weight
    TEST_ASSERT_EQUAL(3000, display->recipeInsertRequiredWeight);
    TEST_ASSERT_EQUAL(0, display->recipeInsertWeight);
}

void test_recipe_auto_advances_to_next_pour_when_flag_is_set()
{
    // one recipe with auto advance enabled, no adjustable parameters
    const Recipe recipes[] = {
        {"", "", 1000, 10, 2, 0, }
    };
    modeRecipes = new ModeRecipes(*weightSensor, *buttons, *display, RECIPES, 3);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_steps_forward_and_back);
    RUN_TEST(test_recipe_switcher);
    RUN_TEST(test_recipe_switcher_select);
    RUN_TEST(test_recipe_configuration_ratio);
    RUN_TEST(test_recipe_configuration_weight);
    RUN_TEST(test_recipe_brewing);
    RUN_TEST(test_adjusting_global_ratio_affects_pour_ratio);
    RUN_TEST(test_recipe_insert_coffee_values);
    RUN_TEST(test_recipe_auto_advances_to_next_pour_when_flag_is_set);
    UNITY_END();
}