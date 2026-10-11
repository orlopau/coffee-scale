#ifndef NATIVE

#include "embedded/sample_queue.h"

#include <Arduino.h>

#define TAG "SAMPLES"

bool SampleQueue::begin()
{
    queue = xQueueCreate(LENGTH, sizeof(Sample));
    if (queue == nullptr)
    {
        ESP_LOGE(TAG, "Cannot create the queue");
        return false;
    }
    return true;
}

void SampleQueue::push(const Sample &sample)
{
    if (xQueueSend(queue, &sample, 0) == pdTRUE)
    {
        if (droppedInARow > 0)
        {
            ESP_LOGW(TAG, "Queue was full, dropped the %u oldest readings", droppedInARow);
            droppedInARow = 0;
        }
        return;
    }

    // full: make room by dropping the oldest
    Sample oldest;
    xQueueReceive(queue, &oldest, 0);
    xQueueSend(queue, &sample, 0);
    droppedInARow++;
}

bool SampleQueue::next(Sample &sample) { return queue != nullptr && xQueueReceive(queue, &sample, 0) == pdTRUE; }

#endif
