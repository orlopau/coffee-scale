/**
 * @file recipes.h
 * @brief Contains initial recipes.
 */

#pragma once
#include "recipe.h"

const uint8_t RECIPE_COUNT = 8;
const Recipe RECIPES[] = {
    {"J. Hofmann Aeropress", 
    "Aeropress recipe by James Hofmann.\nVery tasty!\nnice, nice", 12000, 1667, 3,
        {
            {"Add water wetting all\nthe grounds, then\ninsert plunger 1cm\ninto the brewer.", 1667, 0, 5 * 1000, false},
            {"Gently swirl the\nbrewer.", 0, 0, 30 * 1000, false},
            {"Press the plunger\nall the way in.", 0, 45 * 1000, 0, false},
        }
    },
    {"2222 2", "Aeropress recipe by James Hofmann.", 12000, 1667, 3,
        {
            {"Add water wetting all the grounds,\nthen insert the plunger abaout 1cm into the brewer.", 1667, 0, 2 * 60 * 1000, false},
            {"Gently swirl the brewer.", 0, 0, 30 * 1000, false},
            {"Press the plunger all the way in.", 0, 0, 0, false},
        }
    },
    {"3333333", "Aeropress recipe by James Hofmann.", 12000, 1667, 3,
        {
            {"Add water wetting all the grounds,\nthen insert the plunger abaout 1cm into the brewer.", 1667, 0, 2 * 60 * 1000, false},
            {"Gently swirl the brewer.", 0, 0, 30 * 1000, false},
            {"Press the plunger all the way in.", 0, 0, 0, false},
        }
    },
    {"444444444444 2", "Aeropress recipe by James Hofmann.", 12000, 1667, 3,
        {
            {"Add water wetting all the grounds,\nthen insert the plunger abaout 1cm into the brewer.", 1667, 0, 2 * 60 * 1000, false},
            {"Gently swirl the brewer.", 0, 0, 30 * 1000, false},
            {"Press the plunger all the way in.", 0, 0, 0, false},
        }
    },
    {"5555555555555", "Aeropress recipe by James Hofmann.", 12000, 1667, 3,
        {
            {"Add water wetting all the grounds,\nthen insert the plunger abaout 1cm into the brewer.", 1667, 0, 2 * 60 * 1000, false},
            {"Gently swirl the brewer.", 0, 0, 30 * 1000, false},
            {"Press the plunger all the way in.", 0, 0, 0, false},
        }
    },
    {"666666666666 2", "Aeropress recipe by James Hofmann.", 12000, 1667, 3,
        {
            {"Add water wetting all the grounds,\nthen insert the plunger abaout 1cm into the brewer.", 1667, 0, 2 * 60 * 1000, false},
            {"Gently swirl the brewer.", 0, 0, 30 * 1000, false},
            {"Press the plunger all the way in.", 0, 0, 0, false},
        }
    },
    {"77777777 2", "Aeropress recipe by James Hofmann.", 12000, 1667, 3,
        {
            {"Add water wetting all the grounds,\nthen insert the plunger abaout 1cm into the brewer.", 1667, 0, 2 * 60 * 1000, false},
            {"Gently swirl the brewer.", 0, 0, 30 * 1000, false},
            {"Press the plunger all the way in.", 0, 0, 0, false},
        }
    },
    {"8888888 2", "Aeropress recipe by James Hofmann.", 12000, 1667, 3,
        {
            {"Add water wetting all the grounds,\nthen insert the plunger abaout 1cm into the brewer.", 1667, 0, 2 * 60 * 1000, false},
            {"Gently swirl the brewer.", 0, 0, 30 * 1000, false},
            {"Press the plunger all the way in.", 0, 0, 0, false},
        }
    },
};