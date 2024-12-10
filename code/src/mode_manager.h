#include "mode.h"
#include "display.h"

#define BATTERY_UPDATE_INTERVAL 2000

class ModeManager
{
public:
    ModeManager(Mode *modes[], const int modeCount);
    ~ModeManager(){};
    void update();

private:
    const int modeCount;
    int currentMode;
    bool inModeChange;
    Mode **modes;
    float lastVoltage, lastPercentage;
    long lastBatteryTime;
};