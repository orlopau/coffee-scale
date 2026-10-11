#ifndef NATIVE

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "embedded/sample_queue.h"

/**
 * Reads the load cell in its own task, as soon as the HX711 has a new
 * reading: the HX711 pulls its data pin low when a conversion is ready, and an
 * interrupt on that edge wakes the task. Every reading is timestamped there
 * and put into the queue, where the loop takes it.
 */
class LoadCellTask
{
public:
    /**
     * The longest the task waits for the interrupt. An edge can be missed, e.g.
     * when a reading became ready while the task was reading the one before,
     * and the HX711 then holds its data pin low until it is read.
     */
    static constexpr uint32_t WAIT_MS = 200;

    explicit LoadCellTask(SampleQueue &samples) : samples(samples) {}

    /** Starts the task. False, and logged, if it could not be started. */
    bool begin();

private:
    static void onDataReady(void *task);
    static void run(void *task);
    void loop();

    SampleQueue &samples;
    TaskHandle_t task = nullptr;
};

#endif
