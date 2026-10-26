#include "detectors/shot_detector.h"

#include <math.h>

/** Whether a is later than b, across the wrap of millis(). */
static bool isAfter(uint32_t a, uint32_t b) { return (int32_t)(a - b) > 0; }

ShotEvent ShotDetector::update(const Reading &reading)
{
    if (hasLast && reading.ms - lastMs > MAX_GAP_MS)
    {
        reset();
    }
    hasLast = true;
    lastMs = reading.ms;
    remember(reading);

    if ((state == State::Rising || state == State::Watching) && risen.size() >= HISTORY_READINGS)
    {
        // longer than the history holds
        reject();
        return ShotEvent::None;
    }
    switch (state)
    {
    case State::Waiting:
    case State::Armed:
        rest(reading);
        return state == State::Rising ? rising(reading) : ShotEvent::None;
    case State::Rising:
        risen.push_back(reading);
        return rising(reading);
    case State::Watching:
        risen.push_back(reading);
        return watching(reading);
    case State::Running:
        return running(reading);
    }
    return ShotEvent::None;
}

void ShotDetector::remember(const Reading &reading)
{
    history[next] = {reading.ms, reading.grams, 0};
    next = (next + 1) % HISTORY_READINGS;
    if (count < HISTORY_READINGS)
    {
        count++;
    }
    history[(next - 1 + HISTORY_READINGS) % HISTORY_READINGS].average = fit(reading.ms - AVERAGE_MS, reading.ms).mean;
}

ShotDetector::Line ShotDetector::fit(uint32_t fromMs, uint32_t toMs, bool averaged) const
{
    // newest first: past the readings after toMs, up to fromMs
    int newest = 0;
    while (newest < count && isAfter(back(newest).ms, toMs))
    {
        newest++;
    }
    int n = 0;
    while (newest + n < count && isAfter(back(newest + n).ms, fromMs))
    {
        n++;
    }
    Line line = {0, 0, 0, 0};
    if (n == 0)
    {
        return line;
    }

    // times relative to toMs, small enough for a float
    auto timeOf = [&](int k) { return (int32_t)(back(k).ms - toMs) / 1000.0f; };
    auto valueOf = [&](int k) { return averaged ? back(k).average : back(k).grams; };
    float sumTimes = 0, sumValues = 0;
    for (int k = newest; k < newest + n; k++)
    {
        sumTimes += timeOf(k);
        sumValues += valueOf(k);
    }
    const float meanTime = sumTimes / n;
    line.mean = sumValues / n;
    float sumProducts = 0;
    for (int k = newest; k < newest + n; k++)
    {
        const float dt = timeOf(k) - meanTime;
        line.sumSquaredTimeDeviations += dt * dt;
        sumProducts += dt * (valueOf(k) - line.mean);
    }
    if (line.sumSquaredTimeDeviations > 0)
    {
        line.slope = sumProducts / line.sumSquaredTimeDeviations;
    }
    float sumSquaredResiduals = 0;
    for (int k = newest; k < newest + n; k++)
    {
        const float residual = valueOf(k) - line.mean - line.slope * (timeOf(k) - meanTime);
        sumSquaredResiduals += residual * residual;
    }
    line.sigma = sqrtf(sumSquaredResiduals / n);
    return line;
}

void ShotDetector::rest(const Reading &reading)
{
    const float average = back(0).average;
    const bool rise = state == State::Armed && average > baseline + fmaxf(RISE_SIGMAS * averageSigma, RISE_MIN_GRAMS);
    if (!rise)
    {
        const Line steady = fit(reading.ms - STEADY_MS, reading.ms);
        const bool spansSteadyMs = !isAfter(back(count - 1).ms, reading.ms - STEADY_MS);
        // where the steady weight is against the tare of now
        const float net = steady.mean - (reading.grams - reading.net);
        if (spansSteadyMs && fabsf(steady.slope) <= STEADY_FLOW && fabsf(net) <= ARM_GRAMS && steady.sigma < STEADY_MAX_SIGMA)
        {
            state = State::Armed;
            baseline = steady.mean;
            sigma = steady.sigma;
            averageSigma = fit(reading.ms - STEADY_MS, reading.ms, true).sigma;
        }
        else if (state == State::Waiting)
        {
            risen.clear();
            return;
        }
        // otherwise armed but no longer steady: the values freeze, slow first drops on a noisy scale tilt the stretch

        // at rest against the baseline, steady or not
        if (average <= baseline + DROP_SIGMAS * averageSigma)
        {
            risen.clear();
            return;
        }
    }

    // above rest, maybe already the first drop; a long creep keeps only its end
    if (risen.size() >= HISTORY_READINGS)
    {
        risen.erase(risen.begin());
    }
    risen.push_back(reading);
    if (rise)
    {
        // the walk back to the first drop needs no more than a steady stretch, a long creep would fill the history
        size_t stale = 0;
        while (isAfter(reading.ms - STEADY_MS, risen[stale].ms))
        {
            stale++;
        }
        risen.erase(risen.begin(), risen.begin() + stale);
        state = State::Rising;
        noticedMs = reading.ms;
    }
}

void ShotDetector::trimToFirstDrop()
{
    // from the accepted reading back to the last one at rest against the frozen baseline, the first drop is the one after it
    const float atRest = baseline + DROP_SIGMAS * averageSigma;
    uint32_t firstDrop = back(0).ms;
    for (int k = 1; k < count && !isAfter(risen.front().ms, back(k).ms); k++)
    {
        if (back(k).average <= atRest)
        {
            break;
        }
        firstDrop = back(k).ms;
    }
    size_t before = 0;
    while (isAfter(firstDrop, risen[before].ms))
    {
        before++;
    }
    risen.erase(risen.begin(), risen.begin() + before);
}

ShotEvent ShotDetector::rising(const Reading &reading)
{
    if (reading.ms - noticedMs > FLOOR_WAIT_MS)
    {
        reject();
        return ShotEvent::None;
    }
    if (fit(reading.ms - FLOOR_WINDOW_MS, reading.ms).slope >= FLOOR_FLOW)
    {
        state = State::Watching;
        watchStartMs = reading.ms;
        judgedMs = reading.ms - 1;
    }
    return ShotEvent::None;
}

ShotEvent ShotDetector::watching(const Reading &reading)
{
    // the windows around the watch's first moment fell out of the history
    if (count == HISTORY_READINGS && isAfter(back(count - 1).ms, watchStartMs - SMOOTH_LONG_MS / 2))
    {
        reject();
        return ShotEvent::None;
    }

    // each moment once the window around it is complete, up to the last one whose window ends with the watch
    const uint32_t lastMoment = watchStartMs + WATCH_MS - SMOOTH_LONG_MS / 2;
    uint32_t judged = judgedMs;
    for (int k = 0; k < count && isAfter(back(k).ms, judgedMs); k++)
    {
        const uint32_t moment = back(k).ms;
        if (isAfter(moment + SMOOTH_LONG_MS / 2, reading.ms) || isAfter(moment, lastMoment))
        {
            continue;
        }
        if (!isSmooth(moment))
        {
            reject();
            return ShotEvent::None;
        }
        if (isAfter(moment, judged))
        {
            judged = moment;
        }
    }
    judgedMs = judged;

    const uint32_t watched = reading.ms - watchStartMs;
    if (watched >= FLOOR_FROM_MS && fit(reading.ms - FLOOR_WINDOW_MS, reading.ms).slope < FLOOR_FLOW)
    {
        reject();
        return ShotEvent::None;
    }
    if (watched < WATCH_MS)
    {
        return ShotEvent::None;
    }
    if (fit(watchStartMs - 1, reading.ms).slope > CEILING_FLOW)
    {
        reject();
        return ShotEvent::None;
    }
    trimToFirstDrop();
    highest = risen.front().grams;
    for (const Reading &r : risen)
    {
        highest = fmaxf(highest, r.grams);
    }
    judgeStopFromMs = reading.ms;
    lifted = false;
    waitForRise = false;
    stopCandidate = false;
    state = State::Running;
    return ShotEvent::Accepted;
}

ShotEvent ShotDetector::running(const Reading &reading)
{
    highest = fmaxf(highest, reading.grams);
    if (reading.grams < highest - CUP_LIFTED_GRAMS)
    {
        // the cup is lifted, a candidate was its first reading, partly down; once it is back, the windows wait to be past the lift
        lifted = true;
        waitForRise = true;
        stopCandidate = false;
        judgeStopFromMs = reading.ms + STOP_BEFORE_MS + STOP_WINDOW_MS;
        return ShotEvent::None;
    }
    if (stopCandidate)
    {
        pumpStop = pumpStopCandidate;
        // done: waits for the next tare, waiting's rule
        state = State::Waiting;
        return ShotEvent::PumpStopped;
    }
    if (lifted)
    {
        lifted = false;
        cupBackGrams = reading.grams;
    }
    if (waitForRise && reading.grams < cupBackGrams + CUP_BACK_RISE_GRAMS)
    {
        return ShotEvent::None;
    }
    waitForRise = false;
    if (isAfter(judgeStopFromMs, reading.ms))
    {
        return ShotEvent::None;
    }
    const float earlier = fit(reading.ms - STOP_BEFORE_MS - STOP_WINDOW_MS, reading.ms - STOP_BEFORE_MS).slope;
    const float last = fit(reading.ms - STOP_WINDOW_MS, reading.ms).slope;
    if (earlier < FLOOR_FLOW || last >= STOP_FRACTION * earlier)
    {
        return ShotEvent::None;
    }
    stopCandidate = true;
    pumpStopCandidate = reading.ms - STOP_LOOKBACK_MS;
    return ShotEvent::None;
}

bool ShotDetector::isSmooth(uint32_t ms) const
{
    const Line shortFlow = fit(ms - SMOOTH_SHORT_MS / 2, ms + SMOOTH_SHORT_MS / 2);
    // a single reading has no slope to judge
    if (shortFlow.sumSquaredTimeDeviations <= 0)
    {
        return true;
    }
    const Line longFlow = fit(ms - SMOOTH_LONG_MS / 2, ms + SMOOTH_LONG_MS / 2);
    // noise of a slope through the readings of the short window: σ / √Σ(t − t̄)², t in seconds
    const float limit = SMOOTH_BASE_FLOW + SMOOTH_SIGMAS * sigma / sqrtf(shortFlow.sumSquaredTimeDeviations);
    return fabsf(shortFlow.slope - longFlow.slope) <= limit;
}

void ShotDetector::reject()
{
    state = State::Waiting;
    risen.clear();
}

void ShotDetector::reset()
{
    reject();
    count = 0;
    next = 0;
    hasLast = false;
}
