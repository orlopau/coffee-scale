#pragma once

#include "detectors/lift_detector.h"
#include "weighing/weight_pipeline.h"

/**
 * Lift-to-tare: tares the pipeline when the detector sees a lift, to the
 * weight from before it, with a double beep. Locked while a mode measures,
 * e.g. during a shot.
 */
class LiftTare
{
public:
    /** Two short beeps, unlike auto-tare's single one. */
    static constexpr unsigned int BEEPS = 2;
    static constexpr unsigned int BEEP_MS = 50;

    explicit LiftTare(WeightPipeline &pipeline) : pipeline(pipeline) {}

    /** See LiftDetector::setDepth(), 0 turns lift-to-tare off. */
    void setDepth(float grams) { detector.setDepth(grams); }
    /**
     * Once per loop, after the pipeline's update(). While measuring, the
     * detector starts over, so a dip from then never counts afterwards.
     */
    void update(bool measuring);

private:
    WeightPipeline &pipeline;
    LiftDetector detector;
};
