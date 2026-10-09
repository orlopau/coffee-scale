#pragma once

#include <string>
#include "ui/u8g2_canvas.h"

/**
 * The scale's display without the hardware: u8g2 set up exactly like on the
 * device (SH1107 64x128, full frame buffer, rotated to 128x64), drawing into
 * memory only.
 *
 * Create only one at a time, u8g2 keeps the frame buffer of this display in
 * a static variable.
 */
class VirtualDisplay
{
public:
    VirtualDisplay();

    Canvas &canvas() { return u8g2Canvas; }
    int width() { return u8g2Canvas.width(); }
    int height() { return u8g2Canvas.height(); }

    /** True if the pixel at x, y of the rotated 128x64 screen is lit. */
    bool pixel(int x, int y);
    /** The frame buffer as PNG, each pixel enlarged to scale x scale pixels. */
    std::string png(int scale);

private:
    u8g2_t u8g2;
    U8g2Canvas u8g2Canvas;
};
