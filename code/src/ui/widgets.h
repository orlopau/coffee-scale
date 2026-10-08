#pragma once

#include <stdint.h>
#include "canvas.h"

/**
 * Reusable building blocks for screens. All widgets draw with the canvas'
 * current font unless stated otherwise.
 */
namespace Widgets
{
    /** Draws text horizontally centered with its baseline at y. */
    void textHCentered(Canvas &canvas, const char *text, int y);

    /** Draws text centered horizontally and vertically on the canvas. */
    void textCentered(Canvas &canvas, const char *text);

    /** Draws a centered title with a separator line below it (Font::Small). Returns the y of the line. */
    int titleLine(Canvas &canvas, const char *title);

    /** Draws a bar of count segments at the top with segment index filled. Returns its height. */
    int segmentBar(Canvas &canvas, uint8_t index, uint8_t count);

    /** Draws a framed bar filled to fraction, clamped to 0..1. */
    void progressBar(Canvas &canvas, int x, int y, int w, int h, float fraction);

    /**
     * Draws text word-wrapped (Font::Body). Words are split at spaces. The first line's
     * top is at yTop. Returns the baseline of the last line.
     */
    int textWrapped(Canvas &canvas, const char *text, int yTop, int xLeft, int maxWidth);
}
