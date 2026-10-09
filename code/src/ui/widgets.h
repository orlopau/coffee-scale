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

    /** Draws text (Font::Body) line by line, split at '\n'. */
    void textLines(Canvas &canvas, const char *text);

    /**
     * Draws a list of options below a title line, with the option at index
     * highlighted. Scrolls so the highlighted option stays visible.
     */
    void switcher(Canvas &canvas, const char *title, uint8_t index, uint8_t count, const char *const options[]);

    /** Draws text as a QR code (2 px per module) with its top left at x, y. Returns its size. */
    int qrCode(Canvas &canvas, const char *text, int x, int y);

    /** Baseline that vertically centers text of the current font on y. */
    int centerBaseline(Canvas &canvas, int y);

    /** Blink phase for values being edited: false for 200 ms of every second. */
    bool blinkVisible();
}
