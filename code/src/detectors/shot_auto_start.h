#pragma once

#include <stdint.h>

#include "detectors/shot_detector.h"
#include "mode_manager.h"
#include "modes/mode_espresso.h"
#include "weighing/weight_source.h"

/**
 * Turns the shot detector's events into the espresso mode's actions, if shot
 * detection is on: an accepted shot starts at the first drop minus the
 * offset, on the espresso screen and from the scale mode by switching to it
 * first, and the pump stop stops it. A shot started by a click is accepted
 * too, and then stops at the pump stop, not at the target weight. Off while
 * another mode measures, e.g. a recipe, and in all other modes.
 */
class ShotAutoStart
{
public:
    ShotAutoStart(WeightSource &weights, ModeManager &modes, Mode &scaleMode, ModeEspresso &espresso)
        : weights(weights), modes(modes), scaleMode(scaleMode), espresso(espresso)
    {
    }

    /** Whether shots are detected. Off by default. */
    void setEnabled(bool on) { enabled = on; }
    /** How long before the first drop the pump started. */
    void setOffsetMs(uint32_t offsetMs) { this->offsetMs = offsetMs; }
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

    bool enabled = false;
    uint32_t offsetMs = 0;
    /** Whether the espresso shot ran in the previous update(). */
    bool espressoWasRunning = false;

    void onAccepted();
};
