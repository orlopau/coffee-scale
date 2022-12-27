#include "recipe.h"

uint32_t recipeGetTotalRatio(const Recipe &recipe)
{
    uint32_t totalRatio = 0;
    for (int i = 0; i < recipe.poursCount; i++)
    {
        totalRatio += recipe.pours[i].ratio;
    }
    return totalRatio;
}
