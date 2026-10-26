#pragma once

#include "weighing/weight_source.h"
#include "mode.h"
#include "stopwatch.h"
#include "regression.h"
#include "shot_start.h"

#include <vector>

#define ENCODER_MG_PER_TICK 100

#define MIN_TARGET_WEIGHT_MG 1000
#define MAX_TARGET_WEIGHT_MG 100000

#define REGRESSION_BUFFER_SIZE 50
#define REGRESSION_MAX_TIME 3 * 60 * 1000
#define REGRESSION_GRACE_PERIOD 1000

class ModeEspresso : public Mode
{
public:
    ModeEspresso(WeightSource &weightSource, Stopwatch &stopwatch)
        : weightSource(weightSource), stopwatch(stopwatch),
          targetWeightMg(36 * 1000), approximator(REGRESSION_BUFFER_SIZE), lastEstimatedTime(0){};
    ~ModeEspresso(){};
    void update() override;
    void render(Canvas &canvas) override;
    void enter() override;
    /** Stops a running shot, e.g. one detected by mistake. */
    void leave() override { stopwatch.stop(); }
    bool canSwitchMode() override;
    /** While the shot's stopwatch runs. */
    bool isMeasuring() override { return stopwatch.isRunning(); }
    const char *getName() override;

    /**
     * Starts a shot at startMs, a now() time in the past, without taring, e.g.
     * when the shot detector saw the first drops. The readings since the first
     * drop go into the estimate right away.
     */
    void startShot(unsigned long startMs, ShotStartSource source, uint32_t firstDropMs,
                   const std::vector<Reading> &readings);
    /** Stops the shot as if at stopMs, a now() time in the past, e.g. the pump stop. */
    void stopShotAt(unsigned long stopMs) { stopwatch.stopAt(stopMs); }
    /** An accepted shot isn't stopped by the target weight, its detector stops it. */
    void setAccepted(bool accepted) { this->accepted = accepted; }
    bool isAccepted() const { return accepted; }
    /** Where the last shot's start came from. */
    ShotStartSource getStartSource() const { return startSource; }
    /** The last shot's first drop, 0 if it was started by a click. */
    uint32_t getFirstDropMs() const { return firstDropMs; }

    int32_t getTargetWeightMg() const { return targetWeightMg; }
    /** Estimated time until the target weight is reached. Only meaningful if !isWaitingForEstimate(). */
    int32_t getRemainingTimeMs();
    /** True while there is no usable estimate for the time to finish. */
    bool isWaitingForEstimate();

private:
    WeightSource &weightSource;
    Stopwatch &stopwatch;

    Regression::Approximator approximator;
    int32_t targetWeightMg;
    long lastEstimatedTime;
    ShotStartSource startSource = ShotStartSource::Click;
    bool accepted = false;
    uint32_t firstDropMs = 0;
    /** The newest reading that went into the estimate, so none counts twice. */
    uint32_t lastReadingMs = 0;

    void handleReading(const Reading &reading);
};