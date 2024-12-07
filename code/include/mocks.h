#pragma once

#include "battery.h"
#include "scale.h"
#include "millis.h"
#include "display.h"
#include <math.h>
#include <string.h>

class MockWeightSensor : public WeightSensor
{
public:
    float getWeight() override { return weight; }
    bool isNewWeight() override { return newWeight; }
    void tare() override { weight = 0; }
    void setScale(float scale) override {}
    float getLastWeight() override { return weight; }
    float weight = 0;
    bool newWeight = false;
};

class MockDisplay : public Display
{
public:
    ~MockDisplay()
    {
        delete[] recipeName;
        delete[] recipeDescription;
        delete[] weightConfigHeader;
        delete[] lastCenterText;
        delete[] lastModeText;
    }
    void begin() {}
    void update() {}
    void display(float weight, unsigned long time)
    {
        this->weight = weight;
        this->time = time;
    };
    void promptText(const char *prompt, const char *subtext){};
    void centerText(const char *text, const uint8_t size)
    {
        delete[] lastCenterText;
        lastCenterText = strdup(text);
    };
    void modeSwitcher(const char *current, const uint8_t index, const uint8_t count, float batV, float batPercentage,
                      bool batCharging)
    {
        lastModeText = strdup(current);
    };
    void switcher(const char* title, const uint8_t index, const uint8_t count, const char *options[]) override
    {
        switcherIndex = index;
        switcherCount = count;
    };
    void recipeSummary(const char *name, const char *description, const char* url)
    {
        delete[] recipeName;
        delete[] recipeDescription;
        recipeName = strdup(name);
        recipeDescription = strdup(description);
    };
    void recipeConfigCoffeeWeight(const char *header, unsigned int weightMg, unsigned int waterWeightMl) override
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
    void recipePour(const char *text, int32_t weightToPour, uint64_t timeToFinish, bool isPause, uint8_t pourIndex,
                    uint8_t pours)
    {
        recipeWeightToPourMg = weightToPour;
        recipeTimeToFinishMs = timeToFinish;
        recipeIsPause = isPause;
    };
    void espressoShot(uint32_t currentTimeMs, uint32_t timeToFinishMs, int32_t currentWeightMg, uint32_t targetWeightMg, bool waiting) override
    {
        espressoCurrentTimeMs = currentTimeMs;
        espressoTimeToFinishMs = timeToFinishMs;
        espressoCurrentWeightMg = currentWeightMg;
        espressoTargetWeightMg = targetWeightMg;
    };
    void text(const char *text){};
    void drawTextAutoWrap(const char *text, int yTop){};
    void clear()
    {
        weight = NAN;
        time = -1;
    };

    char *lastCenterText = nullptr;
    char *lastModeText = nullptr;

    float weight = NAN;
    unsigned long time = -1;
    uint8_t switcherIndex = 0xFF;
    uint8_t switcherCount = 0xFF;

    char *recipeName = nullptr;
    char *recipeDescription = nullptr;

    char *weightConfigHeader = nullptr;
    unsigned int weightConfigWeightMg = 0;
    unsigned int weightConfigWaterWeightMl = 0;

    int32_t recipeInsertWeightMg = 0;
    int32_t recipeInsertRequiredWeightMg = 0;

    float ratioCoffee = NAN;
    float ratioWater = NAN;

    uint32_t recipeWeightToPourMg = 0;
    uint64_t recipeTimeToFinishMs = 0;
    bool recipeIsPause = false;

    uint32_t espressoCurrentTimeMs = 0;
    uint32_t espressoTimeToFinishMs = 0;
    int32_t espressoCurrentWeightMg = 0;
    uint32_t espressoTargetWeightMg = 0;
};