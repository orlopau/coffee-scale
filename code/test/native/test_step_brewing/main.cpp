#include "millis.h"
#include "mocks.h"
#include "modes/steps/step_brewing.h"
#include <unity.h>

static MockWeightSensor *weightSensor;
static MockButtons *buttons;
static MockDisplay *display;
static RecipeBrewing *recipeBrewing;
RecipeStepState recipeStepState;

void setUp(void)
{
    weightSensor = new MockWeightSensor();
    buttons = new MockButtons();
    display = new MockDisplay();

    recipeBrewing = new RecipeBrewing(recipeStepState, *display, *buttons, *weightSensor);
    recipeBrewing->enter();
}

void tearDown(void)
{
    delete weightSensor;
    delete buttons;
    delete display;
    delete recipeBrewing;
}

void setRecipe(const Recipe &recipe)
{
    recipeStepState.configRecipe = recipe;
    recipeStepState.originalRecipe = &recipe;
}

void test_can_step_forward(void)
{
    const Recipe singlePourRecipe = {"name1",
                                     "desc1",
                                     3000,
                                     2 * RECIPE_RATIO_MUL,
                                     1,
                                     0,
                                     {
                                         {"pour1", 2 * RECIPE_RATIO_MUL, .timePour = 100, .timePause = 100},
                                     }};
    setRecipe(singlePourRecipe);
    recipeBrewing->update();

    // can only step forward if all pours are done
    // pour not yet done, cant forward
    TEST_ASSERT_FALSE(recipeBrewing->canStepForward());

    // wait for pour done
    sleep_for(201);
    recipeBrewing->update();

    // last pour done, can forward
    TEST_ASSERT_TRUE(recipeBrewing->canStepForward());
}

void test_recipe_brewing(void)
{
    const Recipe recipe = {"name1",
                           "desc1",
                           3000,
                           (2 + 3) * RECIPE_RATIO_MUL,
                           2,
                           static_cast<uint8_t>(AdjustableParameter::COFFEE_WEIGHT) |
                               static_cast<uint8_t>(AdjustableParameter::RATIO),
                           {
                               {"step1", 2 * RECIPE_RATIO_MUL, 500, 300, true},
                               {"step2", 5 * RECIPE_RATIO_MUL, 0, 300, false},
                           }};
    setRecipe(recipe);
    recipeBrewing->update();

    // brewing recipe
    // 2 pours with 2 and 3 times coffee weight as water respectively
    // should show weight to add and time to add it
    TEST_ASSERT_EQUAL(6000, display->recipeWeightToPourMg);
    TEST_ASSERT_EQUAL(500, display->recipeTimeToFinishMs);

    // 500ms pour time
    // after 200ms still 6000mg to pour and ~300ms left
    // still pour time not pause time
    sleep_for(201);
    recipeBrewing->update();
    TEST_ASSERT_LESS_THAN(300, display->recipeTimeToFinishMs);
    TEST_ASSERT_GREATER_THAN(200, display->recipeTimeToFinishMs);
    TEST_ASSERT_FALSE(display->recipeIsPause);

    // wait another ~300ms
    sleep_for(301);
    recipeBrewing->update();
    // now pause has started, 300ms pause time
    TEST_ASSERT_TRUE(display->recipeIsPause);
    TEST_ASSERT_LESS_THAN(300, display->recipeTimeToFinishMs);
    TEST_ASSERT_GREATER_THAN(200, display->recipeTimeToFinishMs);

    // wait another ~300ms
    sleep_for(301);
    recipeBrewing->update();
    // now time should be 0
    TEST_ASSERT_EQUAL(0, display->recipeTimeToFinishMs);

    // next step reached by single click
    buttons->encoderClick = ClickType::SINGLE;
    recipeBrewing->update();
    buttons->encoderClick = ClickType::NONE;
    recipeBrewing->update();

    // verify that second step has started
    // as loadcell always returns 0, total missing weight is now the
    // weight of the first pour + the weight of the second pour = 6000 + 9000 = 15000
    TEST_ASSERT_EQUAL(15000, display->recipeWeightToPourMg);

    // as second pour only contains pause time, should instantly show pause time
    TEST_ASSERT_TRUE(display->recipeIsPause);
    TEST_ASSERT_EQUAL(300, display->recipeTimeToFinishMs);

    // after 300ms, recipe should be done
    sleep_for(301);
    recipeBrewing->update();
    TEST_ASSERT_EQUAL(0, display->recipeTimeToFinishMs);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_can_step_forward);
    RUN_TEST(test_recipe_brewing);
    UNITY_END();
}