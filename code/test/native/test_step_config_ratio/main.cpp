#include "millis.h"
#include "mocks.h"
#include "modes/steps/step_config_ratio.h"
#include <unity.h>

MockButtons *buttons;
MockDisplay *display;
RecipeConfigRatioStep *configRatio;
RecipeStepState recipeStepState;

void setUp(void)
{
    buttons = new MockButtons();
    display = new MockDisplay();
    configRatio = new RecipeConfigRatioStep(recipeStepState, *display, *buttons);
    configRatio->enter();
}

void tearDown(void)
{
    delete buttons;
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
                           .ratio = (2 + 3) * RECIPE_RATIO_MUL,
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
    TEST_ASSERT_EQUAL(1, display->ratioCoffee);
    TEST_ASSERT_EQUAL(5, display->ratioWater);

    // by turning encoder, ratio can be adjusted in steps of 0.1
    buttons->encoderTicks = 1;
    configRatio->update();
    TEST_ASSERT_EQUAL(5.1, display->ratioWater);
    TEST_ASSERT_EQUAL(5.1, recipeStepState.configRecipe.ratio / RECIPE_RATIO_MUL);
    buttons->encoderTicks = -1;
    configRatio->update();
    TEST_ASSERT_EQUAL(4.9, display->ratioWater);
    TEST_ASSERT_EQUAL(4.9, recipeStepState.configRecipe.ratio / RECIPE_RATIO_MUL);

    // ratio can not be reduced below 1:1
    buttons->encoderTicks = -10000;
    configRatio->update();
    TEST_ASSERT_EQUAL(1, display->ratioWater);
    TEST_ASSERT_EQUAL(1, recipeStepState.configRecipe.ratio / RECIPE_RATIO_MUL);

    // maxium ratio is limited by 64
    buttons->encoderTicks = 10000;
    configRatio->update();
    TEST_ASSERT_EQUAL(64 + 5, display->ratioWater);
    TEST_ASSERT_EQUAL(64 + 5, recipeStepState.configRecipe.ratio / RECIPE_RATIO_MUL);

    // return to normal ratio
    buttons->encoderTicks = 0;
    configRatio->update();
    TEST_ASSERT_EQUAL(5, display->ratioWater);
    TEST_ASSERT_EQUAL(5, recipeStepState.configRecipe.ratio / RECIPE_RATIO_MUL);
}

void test_adjusting_ratio_affects_pour_ratios()
{
    const Recipe recipe = {"name1",
                           "desc1",
                           .coffeeWeightMg = 3000,
                           .ratio = (2 + 3) * RECIPE_RATIO_MUL,
                           .poursCount = 2,
                           0,
                           {
                               {"pour1", 2 * RECIPE_RATIO_MUL, .timePour = 100, .timePause = 100},
                               {"pour2", 3 * RECIPE_RATIO_MUL, .timePour = 100, .timePause = 100},
                           }};
    setRecipe(recipe);
    configRatio->update();

    // increase ratio by 5, changing the ratio from 5 to 10 -> 2x
    buttons->encoderTicks = 50;
    configRatio->update();
    TEST_ASSERT_EQUAL(10, recipeStepState.configRecipe.ratio / RECIPE_RATIO_MUL);

    // when config is finished, ratio should be adjusted
    configRatio->exit();
    TEST_ASSERT_EQUAL(10, recipeStepState.configRecipe.ratio / RECIPE_RATIO_MUL);
    // first pour ratio
    TEST_ASSERT_EQUAL(4, recipeStepState.configRecipe.pours[0].ratio / RECIPE_RATIO_MUL);
    // second pour ratio
    TEST_ASSERT_EQUAL(6, recipeStepState.configRecipe.pours[1].ratio / RECIPE_RATIO_MUL);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_adjust_ratio_via_encoder);
    RUN_TEST(test_adjusting_ratio_affects_pour_ratios);
    UNITY_END();
}