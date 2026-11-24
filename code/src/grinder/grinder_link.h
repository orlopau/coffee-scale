#pragma once

#include <stdint.h>

#include "grinder/xenia.h"
#include "mode_manager.h"
#include "modes/mode_espresso.h"
#include "weighing/weight_source.h"

/**
 * Connects the Mahlkönig grinder, which sees the scale as a Xenia espresso
 * machine, to the espresso screen: the grinder's knob press shows the screen
 * and tares the scale, and the shot's end goes back to the grinder with the
 * time the target weight was reached. A shot that ended under the target still
 * counts if a reading reaches it within DRIP_MARGIN_MS, with the shot's time.
 * A knob press in another mode than the scale mode and the espresso screen, or
 * while a shot runs, is aborted.
 */
class GrinderLink
{
public:
    /** How long after the link sees the stopwatch stop under the target the drips may still reach it. */
    static constexpr uint32_t DRIP_MARGIN_MS = 3000;

    GrinderLink(Xenia::Machine &machine, WeightSource &weights, ModeManager &modes, Mode &scaleMode,
                ModeEspresso &espresso)
        : machine(machine), weights(weights), modes(modes), scaleMode(scaleMode), espresso(espresso)
    {
    }

    /** Whether the grinder is served. Off by default; off forgets a shot it watched. */
    void setEnabled(bool on);
    /**
     * Once per loop, after shotAutoStart.update() and before modeManager.update(),
     * with the machine's lock held.
     */
    void update();

private:
    enum class State
    {
        Idle,
        /** The knob was pressed, the stopwatch doesn't run yet. */
        Waiting,
        /** The stopwatch runs. */
        Running,
        /** The stopwatch stopped under the target, the drips may still reach it. */
        Dripping
    };

    Xenia::Machine &machine;
    WeightSource &weights;
    ModeManager &modes;
    Mode &scaleMode;
    ModeEspresso &espresso;

    bool enabled = false;
    State state = State::Idle;
    /** The machine's phase in the previous update(): a knob press is the change to Running. */
    Xenia::Phase lastPhase = Xenia::Phase::Ready;
    /** The shot's time when the stopwatch stopped under the target. */
    uint32_t keptTimeMs = 0;
    /** now() when it did. */
    uint32_t droppedAtMs = 0;

    void onKnobPress();
    void abort(const char *why);
    const char *activeName();
    bool onEspressoScreen() { return modes.activeMode() == &espresso; }
};
