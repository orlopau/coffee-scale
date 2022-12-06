/**
 * @file recipes.h
 * @brief Contains initial recipes.
 */

#pragma once
#include "recipe.h"

#define GRAMS(x) (x * 1000)
#define RATIO(x) (int)(x * 100)
#define SECONDS(x) (x * 1000)
#define MINUTES(x) (x * 60 * 1000)
#define P(x) (static_cast<uint8_t>(x))

const uint8_t RECIPE_COUNT = 8;
const Recipe RECIPES[] = {
    {"Ultimate AeroPress Technique",
     "Aeropress recipe by James Hofmann.\nFirst presented in a video\ncalled\n\"The Ultimate AeroPress Technique\"",
     GRAMS(12),
     RATIO(16.67),
     3,
     P(AdjustableParameter::COFFEE_WEIGHT) | P(AdjustableParameter::RATIO),
     {
         {"Add water wetting all\nthe grounds, then\ninsert plunger 1cm\ninto the brewer.", RATIO(16.67), 0, MINUTES(2), false},
         {"Gently swirl the\nbrewer.", RATIO(16.67), 0, SECONDS(30), false},
         {"Press the plunger\nall the way in.", RATIO(16.67), 0, 0, false},
     }},
};