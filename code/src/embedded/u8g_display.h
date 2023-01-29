#ifndef NATIVE

#pragma once

#include <U8g2lib.h>

#include "user_interface.h"

class U8GDisplay : public Display
{
public:
    U8GDisplay(const uint8_t pin_sda, const uint8_t pin_scl, const u8g2_cb_t *rotation);
    void begin();
    void display(float weight, unsigned long time) override;
    void promptText(const char *prompt, const char *text) override;
    void centerText(const char *text, const uint8_t size) override;
    void modeSwitcher(const char *current, const uint8_t index, const uint8_t count, float batV, float batPercentage, bool batCharging) override;
    void switcher(const char* title, const uint8_t index, const uint8_t count, const char *options[]) override;
    void recipeSummary(const char *name, const char *description, const char* url = nullptr) override;
    void recipeConfigCoffeeWeight(const char *header, unsigned int weightMg, unsigned int waterWeightMl) override;
    void recipeConfigRatio(const char *header, uint32_t coffee, uint32_t water) override;
    void recipeInsertCoffee(int32_t weightMg, uint32_t requiredWeightMg) override;
    void recipePour(const char *text, int32_t weightToPourMg, uint64_t timeToFinishMs, bool isPause, uint8_t pourIndex, uint8_t pours) override;
    void espressoShot(uint32_t currentTimeMs, uint32_t timeToFinishMs, int32_t currentWeightMg, uint32_t targetWeightMg, bool waiting) override;
    void text(const char *text) override;
    void update(){};
    void clear() override;
    void drawOpener();

private:
    U8G2_SH1107_64X128_F_HW_I2C u8g;
    /**
     * @brief Draws the title line at the top of the display.
     *
     * @param title title to draw
     * @return int current y after drawing
     */
    int drawTitleLine(const char *title);
    void drawHCenterText(const char *text, uint8_t y);
    void drawCenterText(const char *text);
    int drawSelectedBar(uint8_t index, uint8_t size);
    int drawQRCode(const char* bytes, uint8_t x, uint8_t y);
    void drawTextAutoWrap(const char *text, int yTop, int xLeft, int maxWidth);
    bool shouldBlinkedBeVisible();
};

#endif