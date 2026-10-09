#ifndef NATIVE

#include <U8g2lib.h>

#include "display.h"
#include "u8g2_canvas.h"
#include "data/bitmaps.h"
#include "constants.h"

static U8G2_SH1107_64X128_F_HW_I2C u8g(U8G2_R1, U8X8_PIN_NONE, PIN_I2C_SCL, PIN_I2C_SDA);
// All frames go through this canvas, so unchanged frames are not resent over I2C.
static U8g2Canvas u8gCanvas(u8g.getU8g2());

Canvas &Display::canvas() { return u8gCanvas; }

void Display::drawOpener()
{
    u8g.clearBuffer();

    u8g.drawXBM(0, 0, chemex_width, chemex_height, chemex_bits);

    u8g.setFont(u8g_font_10x20);
    const static char *textLine1 = "Coffee";
    const static char *textLine2 = "Scale";
    int ascent = u8g.getAscent();
    int width = u8g.getDisplayWidth();
    int height = u8g.getDisplayHeight();

    int remainingCenter = chemex_width + (width - chemex_width) / 2.0;

    int textWidth = u8g.getStrWidth(textLine1);
    int yy = 6 + ascent;
    u8g.drawStr(remainingCenter - textWidth / 2.0, yy, textLine1);
    yy += ascent + 4;
    textWidth = u8g.getStrWidth(textLine2);
    u8g.drawStr(remainingCenter - textWidth / 2.0, yy, textLine2);

    u8g.setFont(u8g_font_7x13);
    ascent = u8g.getAscent();
    const static char *textLineUrl = "orlopau.dev";
    const static char *textLineVersion = FIRMWARE_VERSION;
    textWidth = u8g.getStrWidth(FIRMWARE_VERSION);

    yy += ascent + 5;
    textWidth = u8g.getStrWidth(textLineUrl);
    u8g.drawStr(remainingCenter - textWidth / 2.0, yy, textLineUrl);

    u8g.setFont(u8g_font_6x12);
    ascent = u8g.getAscent();
    textWidth = u8g.getStrWidth(textLineVersion);
    u8g.drawStr(remainingCenter - textWidth / 2.0, height - 2, textLineVersion);

    u8gCanvas.flush();
}

void Display::begin()
{
    u8g.setBusClock(1000000);
    u8g.begin();
    u8gCanvas.invalidate();
}

#endif