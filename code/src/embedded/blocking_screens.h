#ifndef NATIVE

#pragma once

#include <stdint.h>

#include <functional>

#include "canvas.h"

/**
 * Screens shown while something blocks the main loop, e.g. the updater, the
 * Wi-Fi setup or a question at start: they draw and send their frames
 * themselves.
 */
namespace BlockingScreens
{
    /** Waits until the button is released and drops queued clicks, so a press from before doesn't answer. */
    void waitForRelease();

    /** Waits for a fresh click. */
    void waitForClick();

    /** Draws one frame with draw(canvas) and sends it. */
    void show(const std::function<void(Canvas &)> &draw);

    /**
     * Lets the encoder pick one of count options, drawn by draw(canvas,
     * index), until a click. Returns the index clicked; starts at 0.
     */
    uint8_t choose(uint8_t count, const std::function<void(Canvas &, uint8_t)> &draw);
}

#endif
