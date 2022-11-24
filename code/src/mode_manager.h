#include "mode.h"
#include "user_interface.h"

#define MODE_SWITCH_DELAY_MS 2000

class ModeManager
{
public:
    ModeManager(Mode *modes[], const char *modeNames[], const int modeCount, Display &display);
    ~ModeManager(){};
    /**
     * @brief Updates the current mode.
     *
     * @param modeChange 1 for next mode, -1 for previous mode, 0 for no change.
     */
    void update(const int modeChange);

private:
    const int modeCount;
    int currentMode;
    bool inModeChange;
    Mode **modes;
    const char *const *modeNames;
    Display &display;
    unsigned long lastChangeMillis;
};