#ifndef NATIVE

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "weighing/sample.h"

/**
 * The load cell's readings on their way from the load cell task to the loop.
 *
 * If the loop doesn't take them for a while, e.g. while the updater runs,
 * the queue keeps the newest readings and drops the oldest.
 */
class SampleQueue : public SampleSource
{
public:
    /** 3.2 s at the HX711's 10 samples per second. */
    static constexpr UBaseType_t LENGTH = 32;

    /** Creates the queue. False, and logged, if there is no memory for it. */
    bool begin();
    /** Adds a reading, from the load cell task. Drops the oldest if the queue is full. */
    void push(const Sample &sample);
    bool next(Sample &sample) override;

private:
    QueueHandle_t queue = nullptr;
    /** Dropped in the current run of drops, logged when it ends. */
    uint32_t droppedInARow = 0;
};

#endif
