#include "millis.h"
#include "mocks.h"
#include "modes/steps/step_prepare.h"
#include "mock/mock_display.h"
#include <unity.h>

MockWeightSource *weightSource;
RecipePrepare *prepare;
RecipeStepState recipeStepState;

void setUp(void)
{
    Display::reset();
    weightSource = new MockWeightSource();
    prepare = new RecipePrepare(recipeStepState, *weightSource);
    prepare->enter();
}

void tearDown(void)
{
    delete weightSource;
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
    prepare->render(Display::mockCanvas);
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("0.00g/3.0g"));

    weightSource->newReading(1000);
    prepare->update();

    // should show required weight and current weight
    Display::mockCanvas.clear();
    prepare->render(Display::mockCanvas);
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("1000.00g/3.0g"));
}

// right at zero, the noise is either side of it
void test_displays_no_minus_on_zero()
{
    const Recipe recipe = {"name1", "desc1", .coffeeWeightMg = 3000, 1, 0, {{"pour1", 2 * RECIPE_RATIO_MUL, .timePour = 100, .timePause = 100}}};
    setRecipe(recipe);
    weightSource->weight = -0.004f;
    prepare->update();
    prepare->render(Display::mockCanvas);
    TEST_ASSERT_TRUE(Display::mockCanvas.hasText("0.00g/3.0g"));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_displays_current_weight_and_target_coffee_weight);
    RUN_TEST(test_displays_no_minus_on_zero);
    UNITY_END();
}