#include "grinder/grinder_link.h"

#include <math.h>

#include <algorithm>

#include "millis.h"

void GrinderLink::setEnabled(bool on)
{
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
            abort();
        }
        else if (phase != Xenia::Phase::Running)
        {
            // the machine gave up
            state = State::Idle;
        }
        else if (espresso.isMeasuring())
        {
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
            abort();
        }
        else if (espresso.targetReachedMs() >= 0)
        {
            // a drip before the detector's late pump stop can be later than the backdated end
            long timeMs = std::min<long>(espresso.targetReachedMs(), espresso.currentShot().durationMs());
            machine.finish(now(), timeMs, weights.getWeight());
            state = State::Idle;
        }
        else
        {
            // stopped under the target, e.g. at the pump stop: the drips may still get there
            keptTimeMs = espresso.currentShot().durationMs();
            droppedAtMs = now();
            state = State::Dripping;
        }
        break;

    case State::Dripping:
        if (!onEspressoScreen())
        {
            abort();
            break;
        }
        for (const Reading &reading : weights.newReadings())
        {
            // the same comparison the espresso screen stops at
            if ((int32_t)(reading.net * 1000) >= espresso.getTargetWeightMg())
            {
                machine.finish(now(), keptTimeMs, reading.net);
                state = State::Idle;
                return;
            }
        }
        // counted from when the link sees the stop, with Shot Detection on that is ~1 s after the stopwatch's
        if ((uint32_t)(now() - droppedAtMs) >= DRIP_MARGIN_MS)
        {
            abort();
        }
        break;
    }
}

void GrinderLink::onKnobPress()
{
    Mode *active = modes.activeMode();
    if (modes.isMeasuring() && active != &espresso)
    {
        // another mode measures, e.g. the scale mode's stopwatch: leaving would kill its timing,
        // the same rule as ShotAutoStart's
        machine.abort(now());
        return;
    }
    if (active == &scaleMode)
    {
        modes.switchTo(&espresso);
    }
    else if (active != &espresso || espresso.isMeasuring())
    {
        // another mode, the mode switcher or a running shot: not a shot for the grinder
        machine.abort(now());
        return;
    }

    const float grams = machine.brewWeight();
    if (!isnan(grams))
    {
        // anything from the network: lroundf() can't hold every float
        const float maxGrams = MAX_TARGET_WEIGHT_MG / 1000.0f;
        espresso.setTargetWeightMg(lroundf(std::min(grams, maxGrams) * 1000));
    }
    weights.tare();
    state = State::Waiting;
}

void GrinderLink::abort()
{
    machine.abort(now());
    state = State::Idle;
}
