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
    {"name1", "desc1", 3000, 2, static_cast<uint8_t>(AdjustableParameter::COFFEE_WEIGHT) | static_cast<uint8_t>(AdjustableParameter::RATIO), {
                                                                                                                                                                             {"step1", 2 * RECIPE_RATIO_MUL, 500, 300, true},
                                                                                                                                                                             {"step2", 5 * RECIPE_RATIO_MUL, 0, 300, false},
                                                                                                                                                                         }},
    {"name2", "desc2", 2000, 2, 0, {
                                                             {"step1", RECIPE_RATIO_MUL, 0, 2 * 60 * 1000, false},
                                                             {"step2", RECIPE_RATIO_MUL, 0, 30 * 1000, false},
                                                         }},
    {"name3", "desc3", 3000, 2, 0, {
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

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_steps_forward_and_back);
    UNITY_END();
}