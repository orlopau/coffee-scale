#include <unity.h>
#include "mode_default.h"
#include "mode_recipe.h"
#include "globals.h"

void setUp(void)
{
    stopwatch = new Stopwatch();
    loadCell = new MockLoadCell();
    buttons = new MockButtons();
    display = new MockDisplay();
    currSetUp();
}

void tearDown(void)
{
    delete stopwatch;
    delete loadCell;
    delete buttons;
    delete display;
    currTearDown();
}

int main(void)
{
    UNITY_BEGIN();
    run_all_mode_default();
    run_all_mode_recipes();
    UNITY_END();
}