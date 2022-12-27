#pragma once

#include "battery.h"
#include "loadcell.h"
#include "millis.h"
#include "user_interface.h"
#include <math.h>
#include <string.h>

class MockBattery : public Battery
{
public:
    float getVoltage() { return 4.2; };
    float getPercentage() { return 100; };
    bool isCharging() { return false; };
};

class MockWeightSensor : public WeightSensor
{
public:
    float getWeight() override { return weight; }
    bool isNewWeight() override { return newWeight; }
    void tare() override { weight = 0; }
    void setScale(float scale) override {}
    float weight = 0;
    bool newWeight = false;
};

class MockButtons : public UserInput
{
public:
    void update() {}
    long getEncoderTicks() { return encoderTicks; };
    void resetEncoderTicks() { encoderTicks = 0; };
    ClickType consumeEncoderClick()
    {
        ClickType click = encoderClick;
        encoderClick = ClickType::NONE;
        return click;
    }
    ClickType getEncoderClick() { return encoderClick; };
    EncoderDirection getEncoderDirection() { return encoderDirection; };
    void setEncoderTicks(long ticks) { encoderTicks = ticks; };
    ClickType getBootClick() { return bootClick; };
    ClickType encoderClick = ClickType::NONE;
    ClickType bootClick = ClickType::NONE;
    int encoderTicks = 0;
    EncoderDirection encoderDirection = EncoderDirection::NONE;
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
    void switcher(const uint8_t index, const uint8_t count, const char *options[]) override
    {
        switcherIndex = index;
        switcherCount = count;
    };
    void recipeSummary(const char *name, const char *description)
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
    void text(const char *text){};
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
};