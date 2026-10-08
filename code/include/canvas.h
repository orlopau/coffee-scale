#pragma once

#include <stdint.h>

/**
 * Fonts available to screens. Each backend maps these to concrete fonts, so
 * screens and widgets never depend on a specific display library.
 */
enum class Font : uint8_t
{
    Small,    // titles, labels
    Body,     // running text
    Medium,   // emphasized labels
    Large,    // large text
    Number16, // numeric readouts, from small to huge
    Number18,
    Number22,
    Number30,
};

/**
 * Minimal drawing surface used by modes and widgets.
 *
 * A frame is drawn by calling clear(), drawing, then flush(). Text is drawn
 * with its baseline at y and is interpreted as UTF-8. Coordinates may be
 * negative or exceed the canvas, the backend clips.
 */
class Canvas
{
public:
    virtual ~Canvas() {}

    virtual int width() = 0;
    virtual int height() = 0;

    /** Clears the frame buffer. Does not touch the display. */
    virtual void clear() = 0;
    /** Sends the frame buffer to the display. Backends may skip unchanged frames. */
    virtual void flush() = 0;

    virtual void setFont(Font font) = 0;
    /** Ascent of the current font above the baseline, positive. */
    virtual int ascent() = 0;
    /** Descent of the current font below the baseline, zero or negative. */
    virtual int descent() = 0;
    virtual int textWidth(const char *text) = 0;
    virtual void drawText(int x, int y, const char *text) = 0;

    /** 1 draws pixels on, 0 draws pixels off (for inverted content). */
    virtual void setColor(uint8_t color) = 0;
    virtual void drawPixel(int x, int y) = 0;
    virtual void drawLine(int x0, int y0, int x1, int y1) = 0;
    virtual void drawHLine(int x, int y, int w) = 0;
    virtual void drawVLine(int x, int y, int h) = 0;
    virtual void drawBox(int x, int y, int w, int h) = 0;
    virtual void drawFrame(int x, int y, int w, int h) = 0;
};
