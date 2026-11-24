#include "grinder/grinder_link.h"

#include <math.h>

#include <algorithm>

#include "logger.h"
#include "millis.h"

#define TAG "GRINDERLINK"

void GrinderLink::setEnabled(bool on)
{
    if (on != enabled || state != State::Idle)
    {
        LOGI(TAG, "%s%s", on ? "On" : "Off", state != State::Idle ? ", forgets the shot it watched" : "");
    }
    if (on && !enabled)
    {
        // a knob press from before doesn't count
        lastPhase = machine.phase();
    }
    enabled = on;
    state = State::Idle;
}

void GrinderLink::update()
{
    if (!enabled)
    {
        return;
    }

    machine.update(now());

    const Xenia::Phase phase = machine.phase();
    // an abort keeps the machine in Running until the grinder polls: only the change counts as a knob press
    const bool knobPressed = phase == Xenia::Phase::Running && lastPhase != Xenia::Phase::Running;
    lastPhase = phase;

    switch (state)
    {
    case State::Idle:
        if (knobPressed)
        {
            onKnobPress();
        }
        break;

    case State::Waiting:
        if (!onEspressoScreen())
        {
            abort("left the espresso screen before the shot");
        }
        else if (phase != Xenia::Phase::Running)
        {
            // the machine gave up
            LOGI(TAG, "Waiting -> Idle: the machine is %s", Xenia::phaseName(phase));
            state = State::Idle;
        }
        else if (espresso.isMeasuring())
        {
            LOGI(TAG, "Waiting -> Running: the stopwatch runs");
            machine.shotRuns();
            state = State::Running;
        }
        break;

    case State::Running:
        if (espresso.isMeasuring())
        {
            break;
        }
        if (!onEspressoScreen())
        {
            abort("left the espresso screen during the shot");
        }
        else if (espresso.targetReachedMs() >= 0)
        {
            // a drip before the detector's late pump stop can be later than the backdated end
            long timeMs = std::min<long>(espresso.targetReachedMs(), espresso.currentShot().durationMs());
            LOGI(TAG, "Shot ended: target %.1f g reached at %ld ms, shot %ld ms, now %.1f g -> %ld ms",
                 espresso.getTargetWeightMg() / 1000.0, (long)espresso.targetReachedMs(),
                 (long)espresso.currentShot().durationMs(), (double)weights.getWeight(), timeMs);
            machine.finish(now(), timeMs, weights.getWeight());
            state = State::Idle;
        }
        else
        {
            // stopped under the target, e.g. at the pump stop: the drips may still get there
            keptTimeMs = espresso.currentShot().durationMs();
            droppedAtMs = now();
            LOGI(TAG, "Running -> Dripping: stopped at %.1f g of %.1f g after %lu ms, waiting %lu ms for drips",
                 (double)weights.getWeight(), espresso.getTargetWeightMg() / 1000.0, (unsigned long)keptTimeMs,
                 (unsigned long)DRIP_MARGIN_MS);
            state = State::Dripping;
        }
        break;

    case State::Dripping:
        if (!onEspressoScreen())
        {
            abort("left the espresso screen while dripping");
            break;
        }
        for (const Reading &reading : weights.newReadings())
        {
            // the same comparison the espresso screen stops at
            if ((int32_t)(reading.net * 1000) >= espresso.getTargetWeightMg())
            {
                LOGI(TAG, "Drips reached %.1f g after %lu ms -> %lu ms", (double)reading.net,
                     (unsigned long)(uint32_t)(now() - droppedAtMs), (unsigned long)keptTimeMs);
                machine.finish(now(), keptTimeMs, reading.net);
                state = State::Idle;
                return;
            }
        }
        // counted from when the link sees the stop, with Shot Detection on that is ~1 s after the stopwatch's
        if ((uint32_t)(now() - droppedAtMs) >= DRIP_MARGIN_MS)
        {
            abort("the drips stayed under the target");
        }
        break;
    }
}

const char *GrinderLink::activeName()
{
    Mode *active = modes.activeMode();
    if (active == nullptr)
    {
        return "the mode switcher";
    }
    if (active == &scaleMode)
    {
        return "the scale mode";
    }
    if (active == &espresso)
    {
        return "the espresso screen";
    }
    return "another mode";
}

void GrinderLink::onKnobPress()
{
    LOGI(TAG, "Knob press in %s%s", activeName(), modes.isMeasuring() ? ", measuring" : "");
    Mode *active = modes.activeMode();
    if (modes.isMeasuring() && active != &espresso)
    {
        // another mode measures, e.g. the scale mode's stopwatch: leaving would kill its timing,
        // the same rule as ShotAutoStart's
        LOGI(TAG, "Not a shot for the grinder: another mode measures");
        machine.abort(now());
        return;
    }
    if (active == &scaleMode)
    {
        LOGI(TAG, "Switching to the espresso screen");
        modes.switchTo(&espresso);
    }
    else if (active != &espresso || espresso.isMeasuring())
    {
        // another mode, the mode switcher or a running shot: not a shot for the grinder
        LOGI(TAG, "Not a shot for the grinder: %s", active == &espresso ? "a shot runs already" : "not the scale mode");
        machine.abort(now());
        return;
    }

    const float grams = machine.brewWeight();
    if (!isnan(grams))
    {
        // anything from the network: lroundf() can't hold every float
        const float maxGrams = MAX_TARGET_WEIGHT_MG / 1000.0f;
        espresso.setTargetWeightMg(lroundf(std::min(grams, maxGrams) * 1000));
        LOGI(TAG, "Target from the grinder's Brew Weight %.1f g: %.1f g", (double)grams,
             espresso.getTargetWeightMg() / 1000.0);
    }
    else
    {
        LOGI(TAG, "No Brew Weight from the grinder, target stays %.1f g", espresso.getTargetWeightMg() / 1000.0);
    }
    LOGI(TAG, "Tare at %.1f g, Idle -> Waiting for the shot", (double)weights.getWeight());
    weights.tare();
    state = State::Waiting;
}

void GrinderLink::abort(const char *why)
{
    LOGI(TAG, "Abort: %s", why);
    machine.abort(now());
    state = State::Idle;
}
