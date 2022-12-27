/**
 * @file recipes.h
 * @brief Contains initial recipes.
 */

#pragma once
#include "recipe.h"

#define GRAMS(x) (x * 1000)
#define RATIO(x) (int)(x * 10)
#define SECONDS(x) (x * 1000)
#define MINUTES(x) (x * 60 * 1000)
#define P(x) (static_cast<uint8_t>(x))

const uint8_t RECIPE_COUNT = 2;
const Recipe RECIPES[] = {
    {"AeroPress J.H.",
     "Aeropress recipe by\nJames Hofmann.\nFirst presented in a\nvideo called \"The\nUltimate AeroPress\nTechnique\"",
     GRAMS(12),
     3,
     P(AdjustableParameter::COFFEE_WEIGHT) | P(AdjustableParameter::RATIO),
     {
         {"Add water wetting all\nthe grounds, then\ninsert plunger 1cm\ninto the brewer.", RATIO(16.7), 0, MINUTES(2),
          .autoStart = false, .autoAdvance = true},
         {"Gently swirl the\nbrewer.", RATIO(16.7), 0, SECONDS(30), true, true},
         {"Press the plunger\nall the way in.", RATIO(16.7), 0, 0, true, false},
     }},
    {"V60 4-6 Method",
     "V60 recipe by\nTetsu Kasuya.",
     GRAMS(20),
     5,
     P(AdjustableParameter::COFFEE_WEIGHT) | P(AdjustableParameter::RATIO),
     {
         {"First two pours\nadjust sweetness and\nacidity.", RATIO(2.5), 0, SECONDS(45), false, false},
         {"First two pours\nadjust sweetness and\nacidity.", RATIO(6), 0, SECONDS(45), false, false},
         {"Last pours\ncontrol strength.", RATIO(9), 0, SECONDS(45), false, false},
         {"Last pours\ncontrol strength.", RATIO(12), 0, SECONDS(45), false, false},
         {"Last pours\ncontrol strength.", RATIO(15), 0, SECONDS(45), false, false},
     }},
};