/**
 * @file recipes.h
 * @brief Contains initial recipes.
 */

#pragma once
#include "recipe.h"
#include "localization.h"

#define GRAMS(x) (x * 1000)
#define RATIO(x) (int)(x * 10)
#define SECONDS(x) (x * 1000)
#define MINUTES(x) (x * 60 * 1000)
#define P(x) (static_cast<uint8_t>(x))

const uint8_t RECIPE_COUNT = 5;
const Recipe RECIPES[] = {
    {R_AEROPRESS_JH_NAME,
     R_AEROPRESS_JH_DESC,
     "youtu.be/j6VlT_jUVPc",
     GRAMS(12),
     3,
     P(AdjustableParameter::COFFEE_WEIGHT) | P(AdjustableParameter::RATIO),
     {
         {R_AEROPRESS_JH_0, RATIO(16.7), 0, MINUTES(2), .autoStart = false,
          .autoAdvance = true},
         {R_AEROPRESS_JH_1, RATIO(0), 0, SECONDS(30), true, true},
         {R_AEROPRESS_JH_2, RATIO(0), 0, 0, false, true},
     }},
    {R_FRENCH_PRESS_JH_NAME,
     R_FRENCH_PRESS_JH_DESC,
     "youtu.be/st571DYYTR8",
     GRAMS(30),
     4,
     P(AdjustableParameter::COFFEE_WEIGHT) | P(AdjustableParameter::RATIO),
     {
         {R_FRENCH_PRESS_JH_0, RATIO(16.6), SECONDS(15), 0, false, true},
         {R_FRENCH_PRESS_JH_1, RATIO(0), 0, SECONDS(4 * 60), true, true},
         {R_FRENCH_PRESS_JH_2, RATIO(0), 0, SECONDS(5 * 60), true, true},
         {R_FRENCH_PRESS_JH_3, RATIO(0), 0, 0, false, true},
     }},
    {R_V60_KASUYA_NAME,
     R_V60_KASUYA_DESC,
     "youtu.be/wmCW8xSWGZY",
     GRAMS(20),
     5,
     P(AdjustableParameter::COFFEE_WEIGHT) | P(AdjustableParameter::RATIO),
     {
         {R_V60_KASUYA_0, RATIO(2.5), 0, SECONDS(45), false, true},
         {R_V60_KASUYA_1, RATIO(3), 0, SECONDS(45), true, true},
         {R_V60_KASUYA_2, RATIO(3), 0, SECONDS(45), true, true},
         {R_V60_KASUYA_3, RATIO(3), 0, SECONDS(45), true, true},
         {R_V60_KASUYA_4, RATIO(3), 0, SECONDS(45), true, true},
     }},
    {R_V60_RAO_NAME,
     R_V60_RAO_DESC,
     "youtu.be/c0Qe_ASxfNM",
     GRAMS(22),
     5,
     P(AdjustableParameter::COFFEE_WEIGHT) | P(AdjustableParameter::RATIO),
     {
         {R_V60_RAO_0, RATIO(3), SECONDS(45), 0, false, true},
         {R_V60_RAO_1, RATIO(13.4), SECONDS(20), 0, true, true},
         {R_V60_RAO_2, RATIO(0), 0, SECONDS(40), true, true},
         {R_V60_RAO_3, RATIO(0), 0, SECONDS(15), true, true},
         {R_V60_RAO_4, RATIO(0), 0, SECONDS(60), false, true},
     }},
    {R_V60_HARIO_NAME,
     R_V60_HARIO_DESC,
     "",
     GRAMS(12),
     3,
     P(AdjustableParameter::COFFEE_WEIGHT) | P(AdjustableParameter::RATIO),
     {
         {R_V60_HARIO_0, RATIO(3), SECONDS(30), 0, false, true},
         {R_V60_HARIO_1, RATIO(9), 0, 0, false, true},
         {R_V60_HARIO_2, RATIO(0), 0, 0, false, true},
     }},
};