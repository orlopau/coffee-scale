#pragma once

#include <stddef.h>
#include <stdint.h>
#include <functional>

/**
 * Raw load cell samples and button clicks of one recording, kept in RAM until
 * they are uploaded. The buffer is allocated once and never grows: when it is
 * full, or the recording reaches MAX_DURATION_MS, further events are dropped.
 *
 * Times are passed in, so the recorder has no dependency on a clock.
 */
class Recorder
{
public:
    static constexpr unsigned long MAX_DURATION_MS = 300000;
    /** 5 minutes of samples at 10 per second, plus 100 clicks. */
    static constexpr size_t CAPACITY = 3100;

    /** Receives the CSV piece by piece. */
    using Sink = std::function<void(const char *data, size_t length)>;

    explicit Recorder(size_t capacity = CAPACITY);
    ~Recorder();
    Recorder(const Recorder &) = delete;
    Recorder &operator=(const Recorder &) = delete;

    /** False if the buffer could not be allocated, or the capacity is 0. */
    bool isAllocated() const { return events != nullptr; }

    /** Clears the recording and starts a new one at ms. */
    void start(unsigned long ms);
    void addSample(unsigned long ms, long raw);
    void addClick(unsigned long ms);

    bool isFull() const;
    size_t sampleCount() const { return count - clicks; }
    size_t clickCount() const { return clicks; }
    /** Time of the last event since the start, 0 if there is none. */
    unsigned long durationMs() const;
    unsigned long remainingMs() const;

    /**
     * Writes the recording as CSV (see the README) to sink.
     *
     * @return the number of bytes written
     */
    size_t writeCsv(const char *firmware, float gramsPerCount, const Sink &sink) const;

private:
    struct Event
    {
        uint32_t ms;
        int32_t value;
    };

    void add(unsigned long ms, int32_t value);

    Event *events = nullptr;
    size_t capacity = 0;
    size_t count = 0;
    size_t clicks = 0;
    unsigned long startMs = 0;
};
