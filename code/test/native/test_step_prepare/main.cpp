#include "millis.h"
#include "mocks.h"
#include "modes/steps/step_prepare.h"
#include <unity.h>

MockDisplay *display;
MockWeightSensor *weightSensor;
RecipePrepare *prepare;
RecipeStepState recipeStepState;

void setUp(void)
{
    display = new MockDisplay();
    weightSensor = new MockWeightSensor();
    prepare = new RecipePrepare(recipeStepState, *display, *weightSensor);
    prepare->enter();
}

void tearDown(void)
{
    delete weightSensor;
    delete display;
    delete prepare;
}

void setRecipe(const Recipe &recipe)
{
    recipeStepState.configRecipe = recipe;
    recipeStepState.originalRecipe = &recipe;
}

void test_displays_current_weight_and_target_coffee_weight()
{
    const Recipe recipe = {"name1",
                           "desc1",
                           .coffeeWeightMg = 3000,
                           1,
                           0,
                           {
                               {"pour1", 2 * RECIPE_RATIO_MUL, .timePour = 100, .timePause = 100},
                               {"pour2", 3 * RECIPE_RATIO_MUL, .timePour = 100, .timePause = 100},
                           }};
    setRecipe(recipe);
    prepare->update();

    // should show required weight and current weight
    TEST_ASSERT_EQUAL(3000, display->recipeInsertRequiredWeightMg);
    TEST_ASSERT_EQUAL(0, display->recipeInsertWeightMg);

    weightSensor->weight = 1000;
    weightSensor->newWeight = true;
    prepare->update();

    // should show required weight and current weight
    TEST_ASSERT_EQUAL(3000, display->recipeInsertRequiredWeightMg);
    TEST_ASSERT_EQUAL(1000 * 1000, display->recipeInsertWeightMg);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_displays_current_weight_and_target_coffee_weight);
    UNITY_END();
}