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
    const Recipe singlePourRecipe = {
        "name1",
        "desc1",
        "",
        3000,
        1,
        0,
        {
            {"pour1", 2 * RECIPE_RATIO_MUL, .timePour = 100, .timePause = 100, .autoStart = true, .autoAdvance = true},
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
                           "",
                           3000,
                           2,
                           static_cast<uint8_t>(AdjustableParameter::COFFEE_WEIGHT) |
                               static_cast<uint8_t>(AdjustableParameter::RATIO),
                           {
                               {"step1", 2 * RECIPE_RATIO_MUL, 500, 300, .autoStart = true, .autoAdvance = true},
                               {"step2", 3 * RECIPE_RATIO_MUL, 0, 300, .autoStart = true},
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

void test_recipe_auto_advances_step_when_flag_is_set(void)
{
    const Recipe autoAdvanceRecipe = {
        "name1",
        "desc1",
        "",
        3000,
        3,
        0,
        {
            {"pour1", 2 * RECIPE_RATIO_MUL, .timePour = 50, .timePause = 50, .autoStart = true, .autoAdvance = true},
            {"pour2", 2 * RECIPE_RATIO_MUL, .timePour = 50, .timePause = 50},
            {"pour3", 2 * RECIPE_RATIO_MUL, .timePour = 50, .timePause = 50},
        }};
    setRecipe(autoAdvanceRecipe);
    recipeBrewing->update();

    // verify that first step is running
    TEST_ASSERT_EQUAL(0, recipeBrewing->recipePourIndex);
    // wait 100ms for completion
    sleep_for(101);
    recipeBrewing->update();
    // verify that second step has started
    TEST_ASSERT_EQUAL(1, recipeBrewing->recipePourIndex);
    // wait again 100ms for completion
    sleep_for(101);
    recipeBrewing->update();
    // next step does not auto advance, so we should still see the second step
    TEST_ASSERT_EQUAL(1, recipeBrewing->recipePourIndex);
}

void test_recipe_auto_starts_when_flag_is_enbled(void)
{
    const Recipe autoAdvanceRecipe = {
        "name1",
        "desc1",
        "",
        3000,
        3,
        0,
        {
            {"pour1", 2 * RECIPE_RATIO_MUL, .timePour = 50, .timePause = 50, .autoStart = true, .autoAdvance = true},
            {"pour2", 2 * RECIPE_RATIO_MUL, .timePour = 50, .timePause = 50, .autoStart = false, .autoAdvance = true},
            {"pour3", 2 * RECIPE_RATIO_MUL, .timePour = 50, .timePause = 50},
        }};
    setRecipe(autoAdvanceRecipe);
    recipeBrewing->update();

    // verify that first step is running, as it is auto started
    TEST_ASSERT_EQUAL(0, recipeBrewing->recipePourIndex);
    // wait 100ms for completion
    sleep_for(101);
    // auto advances to next step
    recipeBrewing->update();

    // second step does not auto start, recipe should display time to finish
    recipeBrewing->update();
    TEST_ASSERT_EQUAL(50, display->recipeTimeToFinishMs);
    sleep_for(101);
    recipeBrewing->update();
    // after 100ms, still 50ms to go
    TEST_ASSERT_EQUAL(50, display->recipeTimeToFinishMs);

    // only after clicking encoder we should see the next step
    buttons->encoderClick = ClickType::SINGLE;
    recipeBrewing->update();
    buttons->encoderClick = ClickType::NONE;

    // verify that step has started after clicking
    sleep_for(10);
    recipeBrewing->update();
    TEST_ASSERT_LESS_OR_EQUAL(40, display->recipeTimeToFinishMs);
}

void test_recipe_shows_pause_time_when_auto_start_disabled_and_pour_time_0(void)
{
    const Recipe autoAdvanceRecipe = {
        "name1",
        "desc1",
        "",
        3000,
        1,
        0,
        {
            {"pour1", 2 * RECIPE_RATIO_MUL, .timePour = 0, .timePause = 50, .autoStart = false, .autoAdvance = false},
        }};
    setRecipe(autoAdvanceRecipe);
    recipeBrewing->update();

    // verify that the remaining time is shown
    TEST_ASSERT_EQUAL(50, display->recipeTimeToFinishMs);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_can_step_forward);
    RUN_TEST(test_recipe_brewing);
    RUN_TEST(test_recipe_auto_advances_step_when_flag_is_set);
    RUN_TEST(test_recipe_auto_starts_when_flag_is_enbled);
    RUN_TEST(test_recipe_shows_pause_time_when_auto_start_disabled_and_pour_time_0);
    UNITY_END();
}