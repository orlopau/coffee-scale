#include "mode.h"
#include "user_interface.h"
#include "battery.h"

class ModeManager
{
public:
    ModeManager(Mode *modes[], const int modeCount, Display &display, UserInput &input, Battery &battery);
    ~ModeManager(){};
    void update();

private:
    const int modeCount;
    int currentMode;
    bool inModeChange;
    Mode **modes;
    Display &display;
    UserInput &input;
    Battery &battery;
};