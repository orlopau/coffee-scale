#pragma once

#include "auto_tare.h"
#include "mode.h"
#include "stopwatch.h"
#include "weighing/weight_source.h"

class ModeScale : public Mode
{
public:
    ModeScale(WeightSource &weightSource, Stopwatch &stopwatch)
        : weightSource(weightSource), stopwatch(stopwatch){};
    ~ModeScale(){};
    void enter() override;
    void update() override;
    void render(Canvas &canvas) override;
    bool canSwitchMode();
    /** While the stopwatch runs, e.g. for a pour-over by hand. */
    bool isMeasuring() override { return stopwatch.isRunning(); }
    const char* getName();

private:
    WeightSource &weightSource;
    Stopwatch &stopwatch;
    AutoTare *autoTare = new AutoTare(2, 1, 16); // 1g std deviation, 2g tolerance by default
};