#pragma once

#include "mode.h"
#include "display.h"

#define BATTERY_UPDATE_INTERVAL 2000
// Minimum time between two rendered frames (20 fps). Input and weight handling
// in update() still run on every loop, independent of the frame rate.
#define RENDER_INTERVAL_MS 50

class ModeManager
{
public:
    /**
     * @param canvas canvas that modes with rendersToCanvas() draw on. If null, such modes are not rendered.
     */
    ModeManager(Mode *modes[], const int modeCount, Canvas *canvas = nullptr);
    ~ModeManager(){};
    void update();
    void begin();

private:
    const int modeCount;
    int currentMode;
    bool inModeChange;
    Mode **modes;
    float lastVoltage, lastPercentage;
    long lastBatteryTime;

    Canvas *canvas;
    unsigned long lastRenderTime;
    bool renderNow;

    void renderIfDue();
};
