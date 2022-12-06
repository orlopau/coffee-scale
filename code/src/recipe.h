#pragma once

#include <stdint.h>

enum class AdjustableParameter : uint8_t
{
    RATIO = 0x01,
    COFFEE_WEIGHT = 0x02,
    KASUYA_RATIO = 0x04,
    KASUYA_NUM_POURS = 0x08,
};

#define RECIPE_RATIO_MUL 10

/**
 * @brief Defines a single pour.
 *
 */
struct Pour
{
    /// Additional notes for the pour.
    char note[128];
    /// Weight of water per weight of coffee at the current pour step. Must be divided by 10.
    uint32_t ratio;
    /// Time the pour should take in ms.
    uint32_t timePour;
    /// Time to wait after the pour in ms.
    uint32_t timePause;
    /// True if after all time has passed the next pour should start.
    bool autoAdvance;
};

/**
 * @brief Defines a pour over recipe.
 */
struct Recipe
{
    /// The name of the recipe.
    char name[64];
    /// Additional notes for the recipe.
    char note[256];
    /// The grams of coffee used for the recipe.
    uint32_t coffeeWeightMg;
    /// The ratio of coffee to water. Must be divided by 10.
    uint16_t ratio;
    /// The number of pours in the recipe.
    uint8_t poursCount;
    // Adjustable parameters one-hot encoded.
    uint8_t adjustableParameters;
    /// Pours for the recipe.
    Pour pours[10];
};
