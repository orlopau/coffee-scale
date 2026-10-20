#include "detectors/shot_auto_start.h"

void ShotAutoStart::update()
{
    if (modes.isMeasuring())
    {
        detector.reset();
        return;
    }

    // read every loop, turning the target on the espresso screen changes it right away
    detector.setExpectedFlow(espresso.getTargetWeightMg() / 1000.0f / typicalSeconds);
    for (const Reading &reading : weights.newReadings())
    {
        if (detector.update(reading))
        {
            startShot();
        }
    }
}

void ShotAutoStart::startShot()
{
    Mode *active = modes.activeMode();
    if (active == &scaleMode && autoSwitch)
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
