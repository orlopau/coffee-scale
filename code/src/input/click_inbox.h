#pragma once

#include <stdint.h>

#include "input/click.h"

/**
 * Hands the modes one click per loop. The click stays the same for the whole
 * loop, until update() takes the next one. Clicks that waited longer than a
 * second, e.g. while the loop was blocked, are dropped, so they don't act on a
 * screen that has changed meanwhile.
 */
class ClickInbox
{
public:
    static constexpr uint32_t MAX_AGE_MS = 1000;

    explicit ClickInbox(ClickSource &source) : source(source) {}

    /** Forgets the current click and takes the next one that isn't older than MAX_AGE_MS. */
    void update(uint32_t now);
    /** The click of this loop, NONE if there is none. */
    ClickType get() const { return click; }
    /** Returns the click of this loop and clears it. */
    ClickType consume();
    /** Clears the click of this loop and empties the source. */
    void discard();

private:
    ClickSource &source;
    ClickType click = ClickType::NONE;
};
