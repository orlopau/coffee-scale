#pragma once

#include <stdlib.h>
#include "display.h"
#include "mock/mock_canvas.h"

namespace Display
{
    inline RecordingCanvas mockCanvas;

    inline char *lastCenterText = nullptr;
    inline char *lastModeText = nullptr;

    inline float weight = NAN;
    inline unsigned long time = -1;
    inline uint8_t switcherIndex = 0xFF;
    inline uint8_t switcherCount = 0xFF;

    inline char *recipeName = nullptr;
    inline char *recipeDescription = nullptr;

    inline char *weightConfigHeader = nullptr;
    inline unsigned int weightConfigWeightMg = 0;
    inline unsigned int weightConfigWaterWeightMl = 0;

    inline int32_t recipeInsertWeightMg = 0;
    inline int32_t recipeInsertRequiredWeightMg = 0;

    inline float ratioCoffee = NAN;
    inline float ratioWater = NAN;

    inline uint32_t recipeWeightToPourMg = 0;
    inline uint64_t recipeTimeToFinishMs = 0;
    inline bool recipeIsPause = false;

    inline void reset()
    {
        free(lastCenterText);
        free(lastModeText);
        free(recipeName);
        free(recipeDescription);
        free(weightConfigHeader);

        lastCenterText = nullptr;
        lastModeText = nullptr;
        recipeName = nullptr;
        recipeDescription = nullptr;
        weightConfigHeader = nullptr;

        weight = NAN;
        time = -1;
        switcherIndex = 0xFF;
        switcherCount = 0xFF;

        weightConfigWeightMg = 0;
        weightConfigWaterWeightMl = 0;
        recipeInsertWeightMg = 0;
        recipeInsertRequiredWeightMg = 0;
        ratioCoffee = NAN;
        ratioWater = NAN;

        recipeWeightToPourMg = 0;
        recipeTimeToFinishMs = 0;
        recipeIsPause = false;

        mockCanvas.reset();
    }
}