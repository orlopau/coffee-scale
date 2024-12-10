#pragma once
#include "button.h"
#include <cstdio>
#include <math.h>

namespace Display
{
    void begin();
    void drawOpener();
    void display(float weight, unsigned long time);
    void promptText(const char *prompt, const char *subtext);
    void centerText(const char *text, const uint8_t size);
    void modeSwitcher(const char *current, const uint8_t index, const uint8_t count, float batV, float batPercentage, bool batCharging);
    void switcher(const char *title, const uint8_t index, const uint8_t count, const char *options[]);
    void recipeSummary(const char *name, const char *description, const char *url);
    void recipeConfigCoffeeWeight(const char *header, unsigned int weightMg, unsigned int waterWeightMl);
    void recipeConfigRatio(const char *header, uint32_t coffee, uint32_t water);
    void recipeInsertCoffee(int32_t weightMg, uint32_t requiredWeightMg);
    void recipePour(const char *text, int32_t weightToPourMg, uint64_t timeToFinishMs, bool isPause, uint8_t pourIndex, uint8_t pours);
    void espressoShot(uint32_t currentTimeMs, uint32_t timeToFinishMs, int32_t currentWeightMg, uint32_t targetWeightMg, bool waiting);
    void text(const char *text);
    void clear();
};
