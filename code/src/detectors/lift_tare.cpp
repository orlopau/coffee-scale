#include "detectors/lift_tare.h"
#include "interface.h"

void LiftTare::update(bool measuring)
{
    if (measuring)
    {
        detector.reset();
        return;
    }

    for (const Reading &reading : pipeline.newReadings())
    {
        float weightBefore;
        if (detector.update(reading.ms, reading.grams, weightBefore))
        {
            pipeline.tareTo(weightBefore);
            Interface::buzzerPattern(BEEPS, BEEP_MS, BEEP_MS);
        }
    }
}
