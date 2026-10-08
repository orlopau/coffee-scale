#pragma once

#include <string>
#include <vector>
#include "canvas.h"

/**
 * Canvas for native tests. Records the text drawn in the current frame and
 * counts frames, using fixed font metrics (6px per character).
 */
class RecordingCanvas : public Canvas
{
public:
    struct Text
    {
        int x, y;
        std::string text;
    };

    std::vector<Text> texts;
    int boxes = 0;
    int clears = 0;
    int flushes = 0;
    Font font = Font::Body;

    int width() override { return 128; }
    int height() override { return 64; }
    void clear() override
    {
        texts.clear();
        boxes = 0;
        clears++;
    }
    void flush() override { flushes++; }

    void setFont(Font f) override { font = f; }
    int ascent() override { return 10; }
    int descent() override { return -2; }
    int textWidth(const char *text) override { return 6 * (int)std::string(text).size(); }
    void drawText(int x, int y, const char *text) override { texts.push_back({x, y, text}); }

    void setColor(uint8_t color) override {}
    void drawPixel(int x, int y) override {}
    void drawLine(int x0, int y0, int x1, int y1) override {}
    void drawHLine(int x, int y, int w) override {}
    void drawVLine(int x, int y, int h) override {}
    void drawBox(int x, int y, int w, int h) override { boxes++; }
    void drawFrame(int x, int y, int w, int h) override {}

    /** True if some text drawn in the current frame equals text exactly. */
    bool hasText(const char *text) const
    {
        for (const Text &t : texts)
        {
            if (t.text == text)
            {
                return true;
            }
        }
        return false;
    }

    void reset()
    {
        texts.clear();
        boxes = clears = flushes = 0;
        font = Font::Body;
    }
};
