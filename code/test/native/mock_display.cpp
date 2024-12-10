#include <string.h>

#include "mock/mock_display.h"

namespace Display
{
    void begin() {}
    void update() {}
    void display(float weight, unsigned long time)
    {
        Display::weight = weight;
        Display::time = time;
    };
    void promptText(const char *prompt, const char *subtext) {};
    void centerText(const char *text, const uint8_t size)
    {
        delete[] lastCenterText;
        lastCenterText = strdup(text);
    };
    void modeSwitcher(const char *current, const uint8_t index, const uint8_t count, float batV, float batPercentage, bool batCharging)
    {
        lastModeText = strdup(current);
    };
    void switcher(const char *title, const uint8_t index, const uint8_t count, const char *options[])
    {
        switcherIndex = index;
        switcherCount = count;
    };
    void recipeSummary(const char *name, const char *description, const char *url)
    {
        delete[] recipeName;
        delete[] recipeDescription;
        recipeName = strdup(name);
        recipeDescription = strdup(description);
    };
    void recipeConfigCoffeeWeight(const char *header, unsigned int weightMg, unsigned int waterWeightMl)
    {
        weightConfigWeightMg = weightMg;
        weightConfigWaterWeightMl = waterWeightMl;
        delete weightConfigHeader;
        weightConfigHeader = strdup(header);
    };
    void recipeConfigRatio(const char *header, uint32_t coffee, uint32_t water)
    {
        ratioCoffee = coffee;
        ratioWater = water;
    };
    void recipeInsertCoffee(int32_t weightMg, uint32_t requiredWeightMg)
    {
        recipeInsertWeightMg = weightMg;
        recipeInsertRequiredWeightMg = requiredWeightMg;
    };
    void recipePour(const char *text, int32_t weightToPour, uint64_t timeToFinish, bool isPause, uint8_t pourIndex, uint8_t pours)
    {
        recipeWeightToPourMg = weightToPour;
        recipeTimeToFinishMs = timeToFinish;
        recipeIsPause = isPause;
    };
    void espressoShot(uint32_t currentTimeMs, uint32_t timeToFinishMs, int32_t currentWeightMg, uint32_t targetWeightMg, bool waiting)
    {
        espressoCurrentTimeMs = currentTimeMs;
        espressoTimeToFinishMs = timeToFinishMs;
        espressoCurrentWeightMg = currentWeightMg;
        espressoTargetWeightMg = targetWeightMg;
    };
    void text(const char *text) {};
    void drawTextAutoWrap(const char *text, int yTop) {};
    void clear()
    {
        weight = NAN;
        time = -1;
    };
}