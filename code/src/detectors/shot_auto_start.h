#pragma once

#include <stdint.h>

#include "detectors/shot_detector.h"
#include "mode_manager.h"
#include "modes/mode_espresso.h"
#include "weighing/weight_source.h"

/**
 * Starts a shot when the detector sees the first drops: on the espresso
 * screen, and from the scale mode by switching to it first, if switching is
 * on. The start is the first drop minus the offset. Off while a mode
 * measures, e.g. during a shot, and in all other modes.
 */
class ShotAutoStart
{
public:
    ShotAutoStart(WeightSource &weights, ModeManager &modes, Mode &scaleMode, ModeEspresso &espresso)
        : weights(weights), modes(modes), scaleMode(scaleMode), espresso(espresso)
    {
    }

    /** Whether a shot in the scale mode switches to espresso. */
    void setAutoSwitch(bool on) { autoSwitch = on; }
    /** How long before the first drop the pump started. */
    void setOffsetMs(uint32_t offsetMs) { this->offsetMs = offsetMs; }
    /** With the espresso target weight, the flow the detector expects. */
    void setTypicalShotSeconds(float seconds) { typicalSeconds = seconds; }
    /**
     * Once per loop, after the pipeline's update() and before
     * modeManager.update(), from the mode's state of the last loop.
     */
    void update();

private:
    WeightSource &weights;
    ModeManager &modes;
    Mode &scaleMode;
    ModeEspresso &espresso;
    ShotDetector detector;

    bool autoSwitch = true;
    uint32_t offsetMs = 0;
    float typicalSeconds = 30;

    void startShot();
};
