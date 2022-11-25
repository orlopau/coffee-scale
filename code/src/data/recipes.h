/**
 * @file recipes.h
 * @brief Contains initial recipes.
 */

#pragma once
#include "recipe.h"

const Recipe RECIPES[] = {
    {"Ultimate Aeropress Technique", "Aeropress recipe by James Hofmann.",
        {
            {"Add water wetting all the grounds,\nthen insert the plunger abaout 1cm into the brewer.", 11, 0, 2 * 60 * 1000},
            {"Gently swirl the brewer.", 0, 0, 30 * 1000},
            {"Press the plunger all the way in.", 0, 0, 0}
        }
    },
    {"V60 4:6 Method, Default", "V60 recipe by Tetsu Kasuya.",
        {
            {"First 40% of water adjust the balance between sweetness and acidity.", 3, 10 * 1000, 35 * 1000},
            {"First 40% of water adjust the balance between sweetness and acidity.", 3, 10 * 1000, 35 * 1000},
            {"Last 60% is responsible for the body of the coffee.", 3, 10 * 1000, 35 * 1000},
            {"Last 60% is responsible for the body of the coffee.", 3, 10 * 1000, 35 * 1000},
            {"Last 60% is responsible for the body of the coffee.", 3, 10 * 1000, 35 * 1000},
        }
    },
    {"V60 4:6 Method, Sweet", "V60 recipe by Tetsu Kasuya.",
        {
            {"First 40% of water adjust the balance between sweetness and acidity.", 2.5, 10 * 1000, 35 * 1000},
            {"First 40% of water adjust the balance between sweetness and acidity.", 3.5, 10 * 1000, 35 * 1000},
            {"Last 60% is responsible for the body of the coffee.", 3, 10 * 1000, 35 * 1000},
            {"Last 60% is responsible for the body of the coffee.", 3, 10 * 1000, 35 * 1000},
            {"Last 60% is responsible for the body of the coffee.", 3, 10 * 1000, 35 * 1000},
        }
    },
};