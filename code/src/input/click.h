#pragma once

#include <stdint.h>

#include "button.h"

/** A finished click of the encoder's button, with the time it was detected. */
struct ClickEvent
{
    uint32_t ms;
    ClickType type;
};

/** Where the clicks come from, oldest first. */
class ClickSource
{
public:
    virtual ~ClickSource() {}
    /** Takes the next click. False if there is none. */
    virtual bool next(ClickEvent &event) = 0;
};
