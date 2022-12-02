#pragma once

#include "loadcell.h"
#include "user_interface.h"
#include <math.h>
#include <string.h>

class MockLoadCell : public LoadCell
{
public:
    void begin() {}
    void update() {}
    float getWeight() { return weight; }
    bool isNewWeight() { return newWeight; }
    void tare()
    {
        weight = 0;
    }
    void setScale(float scale) {}
    float weight = 0;
    bool newWeight = false;
};

class MockButtons : public UserInput
{
public:
    void update() {}
    long getEncoderTicks() { return encoderTicks; };
    void resetEncoderTicks() { encoderTicks = 0; };
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
        delete[] switcherText;
        delete[] recipeName;
        delete[] recipeDescription;
        delete[] weightConfigHeader;
        delete[] lastCenterText;
    }
    void begin() {}
    void update() {}
    void display(float weight, unsigned long time)
    {
        this->weight = weight;
        this->time = time;
    };
    void promptText(const char *prompt, const char *subtext){};
    void centerText(const char *text, const uint8_t size){
        delete[] lastCenterText;
        lastCenterText = strdup(text);
    };
    void switcher(const char *current, const uint8_t index, const uint8_t count, const char* options[])
    {
        delete[] switcherText;
        switcherText = strdup(current);
    };
    void recipeSummary(const char *name, const char *description)
    {
        delete[] recipeName;
        delete[] recipeDescription;
        recipeName = strdup(name);
        recipeDescription = strdup(description);
    };
    void recipeCoffeeWeightConfig(const char *header, unsigned int weightMg, unsigned int waterWeightMl)
    {
        delete[] weightConfigHeader;
        weightConfigHeader = strdup(header);
        weightConfigWeightMg = weightMg;
        weightConfigWaterWeightMl = waterWeightMl;
    }
    void recipePour(const char* text, uint32_t weightToPour, uint64_t timeToFinish, bool isPause, uint8_t pourIndex, uint8_t pours)
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

    char* lastCenterText = nullptr;

    float weight = NAN;
    unsigned long time = -1;
    char *switcherText = nullptr;
    uint8_t switcherIndex = 0xFF;
    uint8_t switcherCount = 0xFF;

    char *recipeName = nullptr;
    char *recipeDescription = nullptr;

    char *weightConfigHeader = nullptr;
    unsigned int weightConfigWeightMg = 0;
    unsigned int weightConfigWaterWeightMl = 0;

    uint32_t recipeWeightToPourMg = 0;
    uint64_t recipeTimeToFinishMs = 0;
    bool recipeIsPause = false;
};