// The recordings of ADR 2 (docs/adr/0002-shot-detection/) through the pipeline
// and the shot detector, like the main loop: every shot must be found, its
// first drop where the weight leaves rest and its pump stop at the stop
// click, also with the noise of a 5 kg scale added, and nothing else may be.
// The clicks in the recordings mark the pump start and stop; their spikes are
// part of the data, they aren't replayed as clicks.

#include <unity.h>

#include <stdio.h>

#include <algorithm>
#include <filesystem>
#include <vector>

#include "detectors/shot_detector.h"
#include "millis.h"
#include "native/recorded_samples.h"
#include "native/recorded_scale.h"
#include "weighing/weight_pipeline.h"

namespace fs = std::filesystem;

static const fs::path RECORDINGS = fs::path(__FILE__).parent_path().parent_path().parent_path() / "recordings";

static const std::vector<const char *> SHOTS = {
    "2026-10-26/2026-10-26_17-23-18_2.csv", // Shot 1, no stop click
    "2026-10-26/2026-10-26_17-23-19_2.csv", // Shot 2
    "2026-10-26/2026-10-26_17-23-19_3.csv", // Shot 3, the cup lifted after the stop
    "2026-10-26/2026-10-26_17-23-19_4.csv", // Shot 4, a tare before
    "2026-10-26/2026-10-26_17-23-19_5.csv", // Shot 5, a tare before, into a 142 g cup
};
static const int SHOT_1 = 0;

static const std::vector<const char *> BEANS = {
    "2026-10-26/2026-10-26_17-23-18.csv",
    "2026-10-26/2026-10-26_17-23-19.csv",
};

static const std::vector<const char *> LIFTS_TAPS_AND_WARMUP = {
    "2026-10-11_23-03-07.csv", "2026-10-11_23-07-55.csv", "2026-10-11_23-13-38.csv",
    "2026-10-11_23-16-30.csv", "warmup.csv",
};

/** The 5 kg scale at rest. */
static const char *NOISY_SCALE = "2026-10-26_17-56-40.csv";
/** Where in the noise each replay starts, in readings. */
static const std::vector<int> NOISE_OFFSETS = {0, 40, 80, 120, 160};
static const int NO_NOISE = -1;

struct Event
{
    ShotEvent kind;
    /** Accepted: the first drop; PumpStopped: the pump stop. In the recording's ms. */
    unsigned long ms;
};

struct Replay
{
    const char *name;
    int noiseOffset;
    std::vector<unsigned long> clicks;
    /** The readings' times in the recording's ms and their grams, without the noise. */
    std::vector<unsigned long> ms;
    std::vector<float> grams;
    std::vector<Event> events;
};

/** The 5 kg scale's readings at rest, in grams, detrended by their least-squares line. */
static const std::vector<float> &noise()
{
    static std::vector<float> grams;
    if (!grams.empty())
    {
        return grams;
    }
    set_now(1000000);
    RecordedScale scale((RECORDINGS / NOISY_SCALE).string());
    std::vector<double> times;
    std::vector<double> values;
    scale.update();
    while (!scale.isFinished())
    {
        if (scale.isReady())
        {
            times.push_back(scale.elapsed() / 1000.0);
            values.push_back(scale.read() * (double)scale.gramsPerCount());
        }
        sleep_for(1);
    }
    double meanTime = 0, meanValue = 0;
    for (size_t i = 0; i < times.size(); i++)
    {
        meanTime += times[i] / times.size();
        meanValue += values[i] / values.size();
    }
    double covariance = 0, variance = 0;
    for (size_t i = 0; i < times.size(); i++)
    {
        covariance += (times[i] - meanTime) * (values[i] - meanValue);
        variance += (times[i] - meanTime) * (times[i] - meanTime);
    }
    double slope = covariance / variance;
    for (size_t i = 0; i < times.size(); i++)
    {
        grams.push_back((float)(values[i] - meanValue - slope * (times[i] - meanTime)));
    }
    return grams;
}

/** Replays a recording, with the 5 kg noise from reading noiseOffset on, or without. */
static Replay replay(const char *name, int noiseOffset)
{
    const std::vector<float> *added = noiseOffset == NO_NOISE ? nullptr : &noise();

    set_now(1000000);
    RecordedScale scale((RECORDINGS / name).string());
    RecordedSamples samples(scale);
    WeightPipeline pipeline(samples);
    pipeline.setGramsPerCount(scale.gramsPerCount());
    pipeline.setAutoAveraging(1.0f, 64);
    pipeline.setZeroTracking(0.1f);
    ShotDetector detector;

    Replay result = {name, noiseOffset, scale.clicks(), {}, {}, {}};
    scale.update();
    const unsigned long start = now();
    // tared at the start, then at every tare event of the recording
    bool taredAtStart = false;
    size_t nextTare = 0;
    size_t index = 0;
    while (!scale.isFinished())
    {
        pipeline.update();
        for (Reading reading : pipeline.newReadings())
        {
            unsigned long ms = reading.ms - start;
            // not at the first reading like the recorder: that one is in the spike of the
            // click that started the recording, a recording artifact, so tare 1 s in
            if (!taredAtStart && ms >= 1000)
            {
                pipeline.tare();
                taredAtStart = true;
            }
            while (nextTare < scale.tares().size() && scale.tares()[nextTare] <= ms)
            {
                pipeline.tare();
                nextTare++;
            }
            result.ms.push_back(ms);
            result.grams.push_back(reading.grams);
            if (added)
            {
                float n = (*added)[(noiseOffset + index) % added->size()];
                reading.grams += n;
                reading.net += n;
            }
            index++;
            ShotEvent event = detector.update(reading);
            if (event == ShotEvent::Accepted)
            {
                result.events.push_back({event, detector.firstDropMs() - start});
            }
            else if (event == ShotEvent::PumpStopped)
            {
                result.events.push_back({event, detector.pumpStopMs() - start});
            }
        }
        sleep_for(1);
    }
    return result;
}

/**
 * The first drop as in ADR 2's diagrams: the first reading after the pump
 * start click more than 0.1 g above the median of the 10 readings before it,
 * those 10 spanning at most 0.2 g, with the next 15 readings all more than
 * 0.05 g above that median. 0 if there is none.
 */
static unsigned long expectedFirstDrop(const Replay &replay)
{
    const std::vector<float> &grams = replay.grams;
    for (size_t i = 10; i + 15 < grams.size(); i++)
    {
        if (replay.ms[i] <= replay.clicks.front())
        {
            continue;
        }
        std::vector<float> before(grams.begin() + i - 10, grams.begin() + i);
        std::sort(before.begin(), before.end());
        float median = (before[4] + before[5]) / 2;
        if (before.back() - before.front() > 0.2f || grams[i] <= median + 0.1f)
        {
            continue;
        }
        bool staysUp = true;
        for (size_t j = i + 1; j <= i + 15; j++)
        {
            staysUp = staysUp && grams[j] > median + 0.05f;
        }
        if (staysUp)
        {
            return replay.ms[i];
        }
    }
    return 0;
}

static int count(const Replay &replay, ShotEvent kind)
{
    int n = 0;
    for (const Event &event : replay.events)
    {
        n += event.kind == kind;
    }
    return n;
}

/** The ms of the first event of that kind, 0 if there is none. */
static unsigned long first(const Replay &replay, ShotEvent kind)
{
    for (const Event &event : replay.events)
    {
        if (event.kind == kind)
        {
            return event.ms;
        }
    }
    return 0;
}

/** Prints what the replay found, for when an assertion fails. */
static void print(const Replay &replay)
{
    bool isShot = std::find(SHOTS.begin(), SHOTS.end(), replay.name) != SHOTS.end();
    printf("RESULT %-38s noise %4d | expected first drop %6lu | clicks", replay.name, replay.noiseOffset,
           isShot ? expectedFirstDrop(replay) : 0);
    for (unsigned long click : replay.clicks)
    {
        printf(" %lu", click);
    }
    printf(" | events");
    for (const Event &event : replay.events)
    {
        printf(" %s %lu", event.kind == ShotEvent::Accepted ? "first-drop" : "pump-stop", event.ms);
    }
    printf("\n");
}

static void assertFound(const Replay &replay, long earliest, long latest)
{
    TEST_ASSERT_EQUAL_MESSAGE(1, count(replay, ShotEvent::Accepted), replay.name);
    long expected = (long)expectedFirstDrop(replay);
    TEST_ASSERT_NOT_EQUAL_MESSAGE(0, expected, replay.name);
    long firstDrop = (long)first(replay, ShotEvent::Accepted);
    TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(expected + earliest, firstDrop, replay.name);
    TEST_ASSERT_LESS_OR_EQUAL_MESSAGE(expected + latest, firstDrop, replay.name);
}

static void assertPumpStop(const Replay &replay, long within)
{
    TEST_ASSERT_EQUAL_MESSAGE(1, count(replay, ShotEvent::PumpStopped), replay.name);
    TEST_ASSERT_INT32_WITHIN_MESSAGE(within, (long)replay.clicks.back(), (long)first(replay, ShotEvent::PumpStopped),
                                     replay.name);
}

void setUp(void) {}

void tearDown(void) {}

/** Replays and prints each of the recordings, with the noise at each offset, before any assertion. */
static std::vector<Replay> replayAll(const std::vector<const char *> &names, const std::vector<int> &offsets)
{
    std::vector<Replay> replays;
    for (int offset : offsets)
    {
        for (const char *name : names)
        {
            replays.push_back(replay(name, offset));
            print(replays.back());
            // a missing recording replays nothing, and nothing found would pass
            TEST_ASSERT_GREATER_THAN_MESSAGE(100, replays.back().ms.size(), name);
        }
    }
    return replays;
}

void test_every_shot_is_found(void)
{
    for (const Replay &shot : replayAll(SHOTS, {NO_NOISE}))
    {
        assertFound(shot, -1000, 0);
    }
}

static bool isShot1(const Replay &replay) { return replay.name == SHOTS[SHOT_1]; }

void test_every_pump_stop_is_found(void)
{
    for (const Replay &shot : replayAll(SHOTS, {NO_NOISE}))
    {
        if (isShot1(shot))
        {
            // no stop click: at least no pump stop while the pump surely ran
            TEST_ASSERT_EQUAL(1, shot.clicks.size());
            for (const Event &event : shot.events)
            {
                if (event.kind == ShotEvent::PumpStopped)
                {
                    TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(shot.clicks.front() + 25000, event.ms, shot.name);
                }
            }
        }
        else
        {
            TEST_ASSERT_EQUAL_MESSAGE(2, shot.clicks.size(), shot.name);
            assertPumpStop(shot, 500);
        }
    }
}

void test_shots_with_the_noisy_scale(void)
{
    for (const Replay &shot : replayAll(SHOTS, NOISE_OFFSETS))
    {
        assertFound(shot, -1000, 1000);
        if (!isShot1(shot))
        {
            assertPumpStop(shot, 1000);
        }
    }
}

void test_nothing_else_is_a_shot(void)
{
    std::vector<Replay> replays = replayAll(BEANS, {NO_NOISE});
    for (const Replay &noisy : replayAll(BEANS, NOISE_OFFSETS))
    {
        replays.push_back(noisy);
    }
    for (const Replay &other : replayAll(LIFTS_TAPS_AND_WARMUP, {NO_NOISE}))
    {
        replays.push_back(other);
    }
    for (const Replay &replay : replays)
    {
        TEST_ASSERT_EQUAL_MESSAGE(0, replay.events.size(), replay.name);
    }
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_every_shot_is_found);
    RUN_TEST(test_every_pump_stop_is_found);
    RUN_TEST(test_shots_with_the_noisy_scale);
    RUN_TEST(test_nothing_else_is_a_shot);
    return UNITY_END();
}
