#pragma once

#include "weight_sensor.h"
#include "step.h"

class RecipeBrewing : public RecipeStep
{
public:
    RecipeBrewing(RecipeStepState &state, WeightSensor &weightSensor);
    void update() override;
    void render(Canvas &canvas) override;
    void enter() override;
    bool canStepForward() override;
    uint8_t recipePourIndex;

    /** Water still to pour until the end of the current pour, negative if too much was poured. */
    int32_t getRemainingWeightMg() const { return remainingWeightMg; }
    /** Time left in the current pour, or in its pause if isPausing(). */
    uint64_t getRemainingTimeMs() const { return remainingTimeMs; }
    bool isPausing() const { return pausing; }

private:
    RecipeStepState &state;
    WeightSensor &weightSensor;

    unsigned long pourStartMillis = 0;
    bool pourDoneFlag;

    int32_t remainingWeightMg = 0;
    uint64_t remainingTimeMs = 0;
    bool pausing = false;

    void nextPour();
};
