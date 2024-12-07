#include <unity.h>
#include "mocks.h"
#include "stopwatch.h"
#include "modes/mode_recipe.h"
#include "millis.h"
#include "mock/mock_interface.h"

static Stopwatch *stopwatch;
static MockWeightSensor *weightSensor;
static MockDisplay *display;

const Recipe RECIPES[] = {
    {"name1",
     "desc1",
     "url",
     3000,
     2,
     static_cast<uint8_t>(AdjustableParameter::COFFEE_WEIGHT) | static_cast<uint8_t>(AdjustableParameter::RATIO),
     {
         {"step1", 2 * RECIPE_RATIO_MUL, 500, 300, true},
         {"step2", 5 * RECIPE_RATIO_MUL, 0, 300, false},
     }},
    {"name2",
     "desc2",
     "url",
     2000,
     2,
     0,
     {
         {"step1", RECIPE_RATIO_MUL, 0, 2 * 60 * 1000, false},
         {"step2", RECIPE_RATIO_MUL, 0, 30 * 1000, false},
     }},
    {"name3",
     "desc3",
     "url",
     3000,
     2,
     0,
     {
         {"step1", RECIPE_RATIO_MUL, 0, 2 * 60 * 1000, false},
         {"step2", RECIPE_RATIO_MUL, 0, 30 * 1000, false},
     }},
};

ModeRecipes *modeRecipes;

void setUp(void)
{
    stopwatch = new Stopwatch();
    weightSensor = new MockWeightSensor();
    display = new MockDisplay();
    modeRecipes = new ModeRecipes(*weightSensor, *display, RECIPES, 3);
}

void tearDown(void)
{
    delete stopwatch;
    delete weightSensor;
    delete display;
    delete modeRecipes;
}

void test_steps_forward_and_back(void)
{
    TEST_ASSERT_EQUAL(0, modeRecipes->getCurrentStepIndex());
    Interface::encoderClick = ClickType::SINGLE;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(1, modeRecipes->getCurrentStepIndex());
    Interface::encoderClick = ClickType::SINGLE;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(2, modeRecipes->getCurrentStepIndex());
    Interface::encoderClick = ClickType::SINGLE;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(3, modeRecipes->getCurrentStepIndex());
    Interface::encoderClick = ClickType::SINGLE;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(4, modeRecipes->getCurrentStepIndex());
    Interface::encoderClick = ClickType::SINGLE;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(5, modeRecipes->getCurrentStepIndex());
    Interface::encoderClick = ClickType::SINGLE;
    modeRecipes->update();

    // Should not go further, as timer did not expire yet

    TEST_ASSERT_EQUAL(5, modeRecipes->getCurrentStepIndex());
    Interface::encoderClick = ClickType::LONG;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(4, modeRecipes->getCurrentStepIndex());
    Interface::encoderClick = ClickType::LONG;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(3, modeRecipes->getCurrentStepIndex());
    Interface::encoderClick = ClickType::LONG;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(2, modeRecipes->getCurrentStepIndex());
    Interface::encoderClick = ClickType::LONG;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(1, modeRecipes->getCurrentStepIndex());
    Interface::encoderClick = ClickType::LONG;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(0, modeRecipes->getCurrentStepIndex());
    Interface::encoderClick = ClickType::LONG;
    modeRecipes->update();

    TEST_ASSERT_EQUAL(0, modeRecipes->getCurrentStepIndex());
}

void nextStep()
{
    Interface::encoderClick = ClickType::SINGLE;
    modeRecipes->update();
}

void test_issue_25(void)
{
    // verify that when changing a parameter and then going back the parameter is correctly reset
    TEST_ASSERT_EQUAL(0, modeRecipes->getCurrentStepIndex());
    nextStep();
    TEST_ASSERT_EQUAL(1, modeRecipes->getCurrentStepIndex());
    nextStep();
    TEST_ASSERT_EQUAL(2, modeRecipes->getCurrentStepIndex());

    // ratio config step
    // adjust ratio
    Interface::encoderTicks = 10;
    modeRecipes->update();

    // next step
    nextStep();
    TEST_ASSERT_EQUAL(3, modeRecipes->getCurrentStepIndex());

    // go back
    Interface::encoderClick = ClickType::LONG;
    modeRecipes->update();
    TEST_ASSERT_EQUAL(2, modeRecipes->getCurrentStepIndex());

    // go forward again
    nextStep();
    TEST_ASSERT_EQUAL(3, modeRecipes->getCurrentStepIndex());

    // check that ratio is reset to default 21ml
    TEST_ASSERT_EQUAL(21, display->weightConfigWaterWeightMl);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_steps_forward_and_back);
    RUN_TEST(test_issue_25);
    UNITY_END();
}