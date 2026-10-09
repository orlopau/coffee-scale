#include <stdio.h>
#include <string.h>
#include <new>

#include "recorder.h"

// HX711 readings are 24 bit, so this value can't be a sample
static const int32_t CLICK = INT32_MIN;

Recorder::Recorder(size_t capacity)
{
    if (capacity > 0)
    {
        events = new (std::nothrow) Event[capacity];
    }
    this->capacity = events != nullptr ? capacity : 0;
}

Recorder::~Recorder() { delete[] events; }

void Recorder::start(unsigned long ms)
{
    startMs = ms;
    count = 0;
    clicks = 0;
}

void Recorder::addSample(unsigned long ms, long raw) { add(ms, raw); }

void Recorder::addClick(unsigned long ms)
{
    if (!isFull())
    {
        clicks++;
    }
    add(ms, CLICK);
}

void Recorder::add(unsigned long ms, int32_t value)
{
    if (isFull())
    {
        return;
    }
    events[count++] = {static_cast<uint32_t>(ms - startMs), value};
}

bool Recorder::isFull() const { return count >= capacity || durationMs() >= MAX_DURATION_MS; }

unsigned long Recorder::durationMs() const { return count > 0 ? events[count - 1].ms : 0; }

unsigned long Recorder::remainingMs() const
{
    unsigned long duration = durationMs();
    return duration < MAX_DURATION_MS ? MAX_DURATION_MS - duration : 0;
}

size_t Recorder::writeCsv(const char *firmware, float gramsPerCount, const Sink &sink) const
{
    size_t written = 0;
    auto write = [&](const char *data, size_t length) {
        sink(data, length);
        written += length;
    };
    auto writeString = [&](const char *text) { write(text, strlen(text)); };

    char line[64];
    writeString("# coffee-scale recording\n# firmware: ");
    writeString(firmware);
    write(line, snprintf(line, sizeof(line), "\n# grams_per_count: %.9g\nms,event,value\n", gramsPerCount));

    for (size_t i = 0; i < count; i++)
    {
        const Event &event = events[i];
        if (event.value == CLICK)
        {
            write(line, snprintf(line, sizeof(line), "%lu,click,\n", (unsigned long)event.ms));
        }
        else
        {
            write(line, snprintf(line, sizeof(line), "%lu,sample,%ld\n", (unsigned long)event.ms, (long)event.value));
        }
    }
    return written;
}
