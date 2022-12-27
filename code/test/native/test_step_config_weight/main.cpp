#include "millis.h"
#include "mocks.h"
#include "modes/steps/step_config_weight.h"
#include <unity.h>

MockButtons *buttons;
MockDisplay *display;
RecipeConfigWeightStep *configWeight;
RecipeStepState recipeStepState;

void setUp(void)
{
    buttons = new MockButtons();
    display = new MockDisplay();
    configWeight = new RecipeConfigWeightStep(recipeStepState, *display, *buttons);
    configWeight->enter();
}

void tearDown(void)
{
    delete buttons;
    delete display;
    delete configWeight;
}

void setRecipe(const Recipe &recipe)
{
    recipeStepState.configRecipe = recipe;
    recipeStepState.originalRecipe = &recipe;
}

void test_adjust_weight_via_encoder()
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
    configWeight->update();

    // initial coffee weight is 3 grams
    TEST_ASSERT_EQUAL(3000, display->weightConfigWeightMg);
    // 3000mg coffee -> 2 pours with 2 and 3 times coffee weight as water respectively, resulting in 15g water
    TEST_ASSERT_EQUAL(15, display->weightConfigWaterWeightMl);

    // encoder ticks adjust coffee weight
    // turning encoder left should decrease
    buttons->encoderTicks = -1;
    configWeight->update();
    TEST_ASSERT_EQUAL(2000, display->weightConfigWeightMg);
    TEST_ASSERT_EQUAL(2000, recipeStepState.configRecipe.coffeeWeightMg);
    TEST_ASSERT_EQUAL(10, display->weightConfigWaterWeightMl);

    // cant reduce to 0 or below
    buttons->encoderTicks = -3;
    configWeight->update();
    TEST_ASSERT_EQUAL(1000, display->weightConfigWeightMg);
    TEST_ASSERT_EQUAL(1000, recipeStepState.configRecipe.coffeeWeightMg);
    TEST_ASSERT_EQUAL(5, display->weightConfigWaterWeightMl);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_adjust_weight_via_encoder);
    UNITY_END();
}