#ifndef NATIVE

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "weighing/sample.h"

/**
 * Reads the load cell in its own task, as soon as the HX711 has a new
 * reading: the HX711 pulls its data pin low when a conversion is ready, and an
 * interrupt on that edge wakes the task. Every reading is timestamped there
 * and waits in a queue until the loop takes it with next().
 *
 * If the loop doesn't take them for a while, e.g. while the updater runs,
 * the queue keeps the newest readings and drops the oldest.
 */
class LoadCellTask : public SampleSource
{
public:
    /** 3.2 s at the HX711's 10 samples per second. */
    static constexpr UBaseType_t QUEUE_LENGTH = 32;
    /**
     * The longest the task waits for the interrupt. An edge can be missed, e.g.
     * when a reading became ready while the task was reading the one before,
     * and the HX711 then holds its data pin low until it is read.
     */
    static constexpr uint32_t WAIT_MS = 200;

    /** Starts the task. False, and logged, if it could not be started. */
    bool begin();
    bool next(Sample &sample) override;
    /** Readings dropped because the queue was full, since the start. */
    uint32_t droppedSamples() const { return dropped; }

private:
    static void onDataReady(void *task);
    static void run(void *task);
    void loop();
    void push(const Sample &sample);

    QueueHandle_t queue = nullptr;
    TaskHandle_t task = nullptr;
    volatile uint32_t dropped = 0;
    /** Dropped in the current run of drops, logged when it ends. */
    uint32_t droppedInARow = 0;
};

#endif
