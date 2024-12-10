#pragma once

#include "scale.h"
#include "step.h"

class RecipeBrewing : public RecipeStep
{
public:
    RecipeBrewing(RecipeStepState &state, WeightSensor &weightSensor);
    void update() override;
    void enter() override;
    bool canStepForward() override;
    uint8_t recipePourIndex;

private:
    RecipeStepState &state;
    WeightSensor &weightSensor;

    unsigned long pourStartMillis = 0;
    bool pourDoneFlag;

    void nextPour();
};
