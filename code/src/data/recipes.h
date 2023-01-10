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

const uint8_t RECIPE_COUNT = 3;
const Recipe RECIPES[] = {
    {"AeroPress J.H.",
     "Aeropress recipe by James Hofmann. First presented in a video called \"The Ultimate AeroPress Technique\"",
     GRAMS(12),
     3,
     P(AdjustableParameter::COFFEE_WEIGHT) | P(AdjustableParameter::RATIO),
     {
         {"Add water wetting all the grounds, then insert plunger 1cm into the brewer.", RATIO(16.7), 0, MINUTES(2),
          .autoStart = false, .autoAdvance = true},
         {"Gently swirl the brewer.", RATIO(0), 0, SECONDS(30), true, true},
         {"Press the plunger all the way in.", RATIO(0), 0, 0, true, false},
     }},
    {"V60 Kasuya",
     "V60 recipe by Tetsu Kasuya.",
     GRAMS(20),
     5,
     P(AdjustableParameter::COFFEE_WEIGHT) | P(AdjustableParameter::RATIO),
     {
         {"First two pours adjust sweetness and acidity.", RATIO(2.5), 0, SECONDS(45), false, false},
         {"First two pours adjust sweetness and acidity.", RATIO(3), 0, SECONDS(45), false, false},
         {"Last pours control strength.", RATIO(3), 0, SECONDS(45), false, false},
         {"Last pours control strength.", RATIO(3), 0, SECONDS(45), false, false},
         {"Last pours control strength.", RATIO(3), 0, SECONDS(45), false, false},
     }},
    {"V60 Rao",
     "V60 recipe by Scott Rao.",
     GRAMS(22),
     5,
     P(AdjustableParameter::COFFEE_WEIGHT) | P(AdjustableParameter::RATIO),
     {
        {"Bloom the coffee. After pouring, gently stir with a spoon, wetting all grounds.", RATIO(3), SECONDS(45), 0, false, true},
        {"Main pour. Keep kettle at one height, evenly pour the water.", RATIO(16.3), SECONDS(20), 0, true, true},
        {"After pouring, gently stir.", RATIO(0), 0, SECONDS(40), true, true},
        {"Grab the brewer and spin slightly, preventing grounds from sticking to the walls.", RATIO(0), 0, SECONDS(15), true, true},
        {"Wait until the coffee has drained.", RATIO(0), 0, SECONDS(60), true, false},
     }}
};