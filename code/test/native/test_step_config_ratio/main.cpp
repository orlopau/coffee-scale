#include "millis.h"
#include "mocks.h"
#include "mock/mock_interface.h"
#include "modes/steps/step_config_ratio.h"
#include <unity.h>

MockDisplay *display;
RecipeConfigRatioStep *configRatio;
RecipeStepState recipeStepState;

void setUp(void)
{
    display = new MockDisplay();
    configRatio = new RecipeConfigRatioStep(recipeStepState, *display);
    configRatio->enter();
}

void tearDown(void)
{
    delete display;
    delete configRatio;
}

void setRecipe(const Recipe &recipe)
{
    recipeStepState.configRecipe = recipe;
    recipeStepState.originalRecipe = &recipe;
}

void test_adjust_ratio_via_encoder()
{
    const Recipe recipe = {"name1",
                           "desc1",
                           .coffeeWeightMg = 3000,
                           .poursCount = 2,
                           0,
                           {
                               {"pour1", 2 * RECIPE_RATIO_MUL, .timePour = 100, .timePause = 100},
                               {"pour2", 3 * RECIPE_RATIO_MUL, .timePour = 100, .timePause = 100},
                           }};
    setRecipe(recipe);
    configRatio->update();

    // initial ratio is as given in the recipe, i.e. 1 gram of coffee to 5 grams water
    // ratios must be divided by 100 to get the actual ratio
    TEST_ASSERT_EQUAL(10, display->ratioCoffee);
    TEST_ASSERT_EQUAL(50, display->ratioWater);

    // by turning encoder, ratio can be adjusted in steps of 0.1
    Interface::encoderTicks = 1;
    configRatio->update();
    TEST_ASSERT_EQUAL(51, display->ratioWater);
    Interface::encoderTicks = -1;
    configRatio->update();
    TEST_ASSERT_EQUAL(49, display->ratioWater);

    // ratio can not be reduced below 1:1
    Interface::encoderTicks = -10000;
    configRatio->update();
    TEST_ASSERT_EQUAL(10, display->ratioWater);

    // maxium ratio is limited by 64
    Interface::encoderTicks = 10000;
    configRatio->update();
    TEST_ASSERT_EQUAL(640 + 50, display->ratioWater);

    // return to normal ratio
    Interface::encoderTicks = 0;
    configRatio->update();
    TEST_ASSERT_EQUAL(50, display->ratioWater);
}

void test_adjusting_ratio_affects_pour_ratios()
{
    const Recipe recipe = {"name1",
                           "desc1",
                           .coffeeWeightMg = 3000,
                           .poursCount = 2,
                           0,
                           {
                               {"pour1", 2 * RECIPE_RATIO_MUL, .timePour = 100, .timePause = 100},
                               {"pour2", 3 * RECIPE_RATIO_MUL, .timePour = 100, .timePause = 100},
                           }};
    setRecipe(recipe);
    configRatio->update();

    // increase ratio by 5, changing the ratio from 5 to 10 -> 2x
    Interface::encoderTicks = 50;
    configRatio->update();

    // when config is finished, ratio should be adjusted
    configRatio->exit();
    TEST_ASSERT_EQUAL(100, recipeGetTotalRatio(recipeStepState.configRecipe));
    // first pour ratio
    TEST_ASSERT_EQUAL(40, recipeStepState.configRecipe.pours[0].ratio);
    // second pour ratio
    TEST_ASSERT_EQUAL(60, recipeStepState.configRecipe.pours[1].ratio);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_adjust_ratio_via_encoder);
    RUN_TEST(test_adjusting_ratio_affects_pour_ratios);
    UNITY_END();
}