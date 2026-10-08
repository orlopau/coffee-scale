#include <string.h>

#include "ui/widgets.h"

namespace Widgets
{
    void textHCentered(Canvas &canvas, const char *text, int y)
    {
        canvas.drawText((int)(canvas.width() / 2.0 - canvas.textWidth(text) / 2.0), y, text);
    }

    void textCentered(Canvas &canvas, const char *text)
    {
        textHCentered(canvas, text, (int)(canvas.height() / 2.0 + canvas.ascent() / 2.0));
    }

    int titleLine(Canvas &canvas, const char *title)
    {
        static const int PADDING = 4;

        canvas.setFont(Font::Small);
        int y = canvas.ascent() - canvas.descent();
        textHCentered(canvas, title, y);
        y += PADDING;
        canvas.drawHLine(0, y, canvas.width());
        return y;
    }

    int segmentBar(Canvas &canvas, uint8_t index, uint8_t count)
    {
        static const int HEIGHT = 4;

        if (count == 0)
        {
            return HEIGHT;
        }

        int width = canvas.width();
        int segmentWidth = width / count;
        canvas.drawFrame(0, 0, width, HEIGHT);
        for (int i = 1; i < count; i++)
        {
            canvas.drawVLine(i * segmentWidth, 0, HEIGHT);
        }
        canvas.drawBox(index * segmentWidth, 0, segmentWidth, HEIGHT);
        return HEIGHT;
    }

    void progressBar(Canvas &canvas, int x, int y, int w, int h, float fraction)
    {
        if (!(fraction > 0)) // also catches NaN
        {
            fraction = 0;
        }
        else if (fraction > 1)
        {
            fraction = 1;
        }

        canvas.drawFrame(x, y, w, h);
        int filled = (int)(fraction * w);
        if (filled > 0)
        {
            canvas.drawBox(x, y, filled, h);
        }
    }

    int textWrapped(Canvas &canvas, const char *text, int yTop, int xLeft, int maxWidth)
    {
        canvas.setFont(Font::Body);
        int lineHeight = canvas.ascent() - canvas.descent();
        int spaceWidth = canvas.textWidth(" ");

        int line = yTop + lineHeight;
        int x = xLeft;

        // words longer than this cannot fit a line on the display anyway
        char word[64];
        const char *p = text;
        while (*p != '\0')
        {
            while (*p == ' ')
            {
                p++;
            }
            if (*p == '\0')
            {
                break;
            }

            size_t len = strcspn(p, " ");
            size_t copied = len < sizeof(word) - 1 ? len : sizeof(word) - 1;
            memcpy(word, p, copied);
            word[copied] = '\0';
            p += len;

            int wordWidth = canvas.textWidth(word);
            if (x + wordWidth > maxWidth)
            {
                x = xLeft;
                line += lineHeight;
            }
            canvas.drawText(x, line, word);
            x += wordWidth + spaceWidth;
        }

        return line;
    }
}
