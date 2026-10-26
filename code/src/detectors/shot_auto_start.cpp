#include "detectors/shot_auto_start.h"

void ShotAutoStart::update()
{
    const bool shotRuns = espresso.isMeasuring();
    const bool shotRanBefore = espressoWasRunning;
    espressoWasRunning = shotRuns;

    if (!enabled)
    {
        detector.reset();
        return;
    }

    // another mode than espresso measures, e.g. a recipe
    if (modes.isMeasuring() && modes.activeMode() != &espresso)
    {
        detector.reset();
        return;
    }

    if (shotRanBefore && !shotRuns)
    {
        // a click, the target weight or leaving ended the shot, accepted or not: it waits for the next tare
        detector.reset();
    }
    else if (espresso.isAccepted() && shotRuns && !detector.isShotRunning())
    {
        // the detector lost the shot, e.g. after a gap in the readings: the target weight stops it again
        espresso.setAccepted(false);
    }

    for (const Reading &reading : weights.newReadings())
    {
        const ShotEvent event = detector.update(reading);
        if (event == ShotEvent::Accepted)
        {
            onAccepted();
        }
        else if (event == ShotEvent::PumpStopped && espresso.isMeasuring() && espresso.isAccepted())
        {
            espresso.stopShotAt(detector.pumpStopMs());
        }
    }
}

void ShotAutoStart::onAccepted()
{
    if (espresso.isMeasuring())
    {
        // started by a click: the detector takes it over
        espresso.setAccepted(true);
        return;
    }

    Mode *active = modes.activeMode();
    if (active == &scaleMode)
    {
        modes.switchTo(&espresso);
    }
    else if (active != &espresso)
    {
        return;
    }

    const uint32_t firstDrop = detector.firstDropMs();
    // an offset longer than the time since the start would wrap around
    const uint32_t start = firstDrop > offsetMs ? firstDrop - offsetMs : 0;
    espresso.startShot(start, ShotStartSource::FirstDrop, firstDrop, detector.rise());
}
