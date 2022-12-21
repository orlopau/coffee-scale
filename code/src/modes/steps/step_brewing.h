#pragma once

#include "loadcell.h"
#include "step.h"
#include "user_interface.h"

class RecipeBrewing : public RecipeStep
{
public:
    RecipeBrewing(RecipeStepState &state, Display &display, UserInput &input, WeightSensor &weightSensor);
    void update() override;
    void enter() override;
    bool canStepForward() override;
    uint8_t recipePourIndex;

private:
    RecipeStepState &state;
    Display &display;
    UserInput &input;
    WeightSensor &weightSensor;

    enum BrewState
    {
        /// @brief Brew is not started yet.
        STOPPED,
        /// @brief Brew is in progress.
        POUR,
        /// @brief Brew is paused.
        PAUSE,
        /// @brief Brew is done.
        DONE,
    };
    unsigned long pourStartMillis = 0;
    bool pourDoneFlag;
};
