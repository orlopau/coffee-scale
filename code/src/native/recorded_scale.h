#pragma once

#include <string>
#include <utility>
#include <vector>

/**
 * A recording of the real scale's load cell (see the README), replayed: like
 * SimulatedScale, but the readings come from a CSV file recorded on the
 * scale.
 *
 * The replay starts with the first call of update(), isReady() or read().
 * From then on, each sample becomes ready when as much time has passed on
 * now() as when it was recorded, so it follows the virtual clock of native
 * builds.
 */
class RecordedScale
{
public:
    /** Loads the recording. Throws std::runtime_error("<path>:<line>: <reason>") if it is malformed. */
    explicit RecordedScale(const std::string &path);

    /** Starts the replay if it hasn't started yet. */
    void update();
    /** True if a sample is due that hasn't been read yet. */
    bool isReady();
    /**
     * The latest due sample. Like the HX711, which only holds the newest
     * reading, due samples before it are skipped. Returns the last read value
     * if none is due.
     */
    long read();
    /** True once the last sample was read. */
    bool isFinished() const { return next >= samples.size(); }

    /** Time since the replay started, comparable with the recorded times. */
    unsigned long elapsed() const;
    /** Times of the clicks, since the start of the recording. */
    const std::vector<unsigned long> &clicks() const { return clickTimes; }
    float gramsPerCount() const { return scale; }
    const std::string &firmware() const { return firmwareVersion; }

private:
    std::vector<std::pair<unsigned long, long>> samples;
    std::vector<unsigned long> clickTimes;
    float scale = 0;
    std::string firmwareVersion;

    bool started = false;
    unsigned long startTime = 0;
    size_t next = 0;
    long last = 0;
};
