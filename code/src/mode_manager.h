#include "mode.h"
#include "display.h"

#define BATTERY_UPDATE_INTERVAL 2000

class ModeManager
{
public:
    ModeManager(Mode *modes[], const int modeCount, Display &display);
    ~ModeManager(){};
    void update();

private:
    const int modeCount;
    int currentMode;
    bool inModeChange;
    Mode **modes;
    Display &display;
    float lastVoltage, lastPercentage;
    long lastBatteryTime;
};