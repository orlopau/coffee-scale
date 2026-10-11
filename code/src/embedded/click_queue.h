#ifndef NATIVE

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "input/click.h"

/**
 * The button's clicks on their way from the button task to the loop. If the
 * loop doesn't take them for a while, the queue keeps the newest and drops the
 * oldest.
 */
class ClickQueue : public ClickSource
{
public:
    static constexpr UBaseType_t LENGTH = 8;

    /** Creates the queue. False, and logged, if there is no memory for it. */
    bool begin();
    /** Adds a click, from the button task. Drops the oldest if the queue is full. */
    void push(const ClickEvent &event);
    bool next(ClickEvent &event) override;

private:
    QueueHandle_t queue = nullptr;
    /** Dropped in the current run of drops, logged when it ends. */
    uint32_t droppedInARow = 0;
};

#endif
