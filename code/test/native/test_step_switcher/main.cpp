#include "millis.h"
#include "mocks.h"
#include "modes/steps/step_switcher.h"
#include <unity.h>

MockButtons *buttons;
MockDisplay *display;
RecipeSwitcherStep *switcherStep;
RecipeStepState recipeStepState;

const uint8_t RECIPE_COUNT = 3;
const Recipe RECIPES[] = {
    {"r1", "note1", .coffeeWeightMg = 3000, 0, 0, {}},
    {"r2", "note2", .coffeeWeightMg = 3000, 0, 0, {}},
    {"r3", "note3", .coffeeWeightMg = 3000, 0, 0, {}},
};

void setUp(void)
{
    buttons = new MockButtons();
    display = new MockDisplay();
    switcherStep = new RecipeSwitcherStep(recipeStepState, *display, *buttons, RECIPES, RECIPE_COUNT);
    switcherStep->enter();
}

void tearDown(void)
{
    delete buttons;
    delete display;
    delete switcherStep;
}

void test_encoder_rotation_changes_chosen_recipe()
{
    switcherStep->update();

    // initial recipe is number 0
    TEST_ASSERT_EQUAL(0, display->switcherIndex);

    // turning encoder to left should remain at 0
    buttons->encoderDirection = EncoderDirection::CCW;
    switcherStep->update();
    TEST_ASSERT_EQUAL(0, display->switcherIndex);

    // turning encoder to right should change to 1
    buttons->encoderDirection = EncoderDirection::CW;
    switcherStep->update();
    TEST_ASSERT_EQUAL(1, display->switcherIndex);

    // turning encoder to right should change to 2
    buttons->encoderDirection = EncoderDirection::CW;
    switcherStep->update();
    TEST_ASSERT_EQUAL(2, display->switcherIndex);

    // turning encoder to right should remain at 2
    buttons->encoderDirection = EncoderDirection::CW;
    switcherStep->update();
    TEST_ASSERT_EQUAL(2, display->switcherIndex);

    // turning encoder to left should change to 1
    buttons->encoderDirection = EncoderDirection::CCW;
    switcherStep->update();
    TEST_ASSERT_EQUAL(1, display->switcherIndex);
}

void test_switcher_sets_recipe_state_to_chosen_recipe(void)
{
    switcherStep->recipeIndex = 1;
    switcherStep->update();
    switcherStep->exit();

    TEST_ASSERT_EQUAL_STRING(RECIPES[1].name, recipeStepState.configRecipe.name);
    TEST_ASSERT_EQUAL_STRING(RECIPES[1].name, recipeStepState.originalRecipe->name);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_encoder_rotation_changes_chosen_recipe);
    RUN_TEST(test_switcher_sets_recipe_state_to_chosen_recipe);
    UNITY_END();
}