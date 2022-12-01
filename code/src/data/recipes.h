/**
 * @file recipes.h
 * @brief Contains initial recipes.
 */

#pragma once
#include "recipe.h"

const uint8_t RECIPE_COUNT = 2;
const Recipe RECIPES[] = {
    {"Ultimate Aeropress Technique", "Aeropress recipe by James Hofmann.", 12000, 1667, 3,
        {
            {"Add water wetting all the grounds,\nthen insert the plunger abaout 1cm into the brewer.", 1667, 0, 2 * 60 * 1000, false},
            {"Gently swirl the brewer.", 0, 0, 30 * 1000, false},
            {"Press the plunger all the way in.", 0, 0, 0, false},
        }
    },
};