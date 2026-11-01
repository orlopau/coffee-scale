#include "ui/shot_graph.h"

#include <math.h>
#include <stdio.h>

#include "ui/widgets.h"

namespace ShotGraph
{
    /** The row of the time axis, the graph is above it, the labels below. */
    static const int AXIS_Y = 53;
    static const uint32_t TICK_MS = 10000;
    static const int TICK_HEIGHT = 2;
    /** Room after the longer shot's end, so it isn't on the edge. */
    static const uint32_t END_MARGIN_MS = 3000;
    /** Room above the higher shot. */
    static const float HEADROOM = 1.15f;

    /** The highest point, normally the yield; higher if the cup was lifted at the end. */
    static float highestGrams(const ShotCurve &curve)
    {
        float highest = 0;
        for (size_t i = 0; i < curve.size(); i++)
        {
            if (curve.gramsAt(i) > highest)
            {
                highest = curve.gramsAt(i);
            }
        }
        return highest;
    }

    struct Axes
    {
        int width;
        uint32_t maxMs;
        float maxGrams;

        int x(uint32_t ms) const { return (int)((uint64_t)ms * (width - 1) / maxMs); }
        int y(float grams) const { return AXIS_Y - (int)lroundf(grams / maxGrams * AXIS_Y); }
    };

    void render(Canvas &canvas, const ShotCurve &shot, const ShotCurve *last)
    {
        const uint32_t lastMs = last ? last->durationMs() : 0;
        const float lastGrams = last ? highestGrams(*last) : 0;
        const float shotGrams = highestGrams(shot);
        Axes axes;
        axes.width = canvas.width();
        axes.maxMs = (shot.durationMs() > lastMs ? shot.durationMs() : lastMs) + END_MARGIN_MS;
        axes.maxGrams = HEADROOM * (shotGrams > lastGrams ? shotGrams : lastGrams);
        if (axes.maxGrams <= 0)
        {
            axes.maxGrams = 1;
        }

        // the last shot dotted, every other point
        if (last)
        {
            for (size_t i = 0; i < last->size(); i += 2)
            {
                canvas.drawPixel(axes.x(i * ShotCurve::INTERVAL_MS), axes.y(last->gramsAt(i)));
            }
        }

        for (size_t i = 1; i < shot.size(); i++)
        {
            canvas.drawLine(axes.x((i - 1) * ShotCurve::INTERVAL_MS), axes.y(shot.gramsAt(i - 1)),
                            axes.x(i * ShotCurve::INTERVAL_MS), axes.y(shot.gramsAt(i)));
        }

        canvas.drawHLine(0, AXIS_Y, axes.width);
        for (uint32_t ms = TICK_MS; ms < axes.maxMs; ms += TICK_MS)
        {
            canvas.drawVLine(axes.x(ms), AXIS_Y - TICK_HEIGHT, TICK_HEIGHT);
        }

        // the yield, dashed across, its label on the line with the curve cleared behind it
        const int yieldY = axes.y(shot.yieldGrams());
        for (int x = 0; x < axes.width; x += 4)
        {
            canvas.drawHLine(x, yieldY, 2);
        }
        char buffer[16];
        canvas.setFont(Font::Small);
        snprintf(buffer, sizeof(buffer), "%.1fg", shot.yieldGrams());
        const int yieldBaseline = Widgets::centerBaseline(canvas, yieldY);
        canvas.setColor(0);
        canvas.drawBox(0, yieldBaseline - canvas.ascent() - 1, canvas.textWidth(buffer) + 2,
                       canvas.ascent() - canvas.descent() + 2);
        canvas.setColor(1);
        canvas.drawText(0, yieldBaseline, buffer);

        // under the axis: 0 at the start, the time at the shot's end, kept on screen
        const int labelBaseline = canvas.height() - 1;
        canvas.drawText(0, labelBaseline, "0");
        snprintf(buffer, sizeof(buffer), "%.1fs", shot.durationMs() / 1000.0f);
        const int timeWidth = canvas.textWidth(buffer);
        int timeX = axes.x(shot.durationMs()) - timeWidth / 2;
        if (timeX + timeWidth > axes.width)
        {
            timeX = axes.width - timeWidth;
        }
        if (timeX < 0)
        {
            timeX = 0;
        }
        canvas.drawText(timeX, labelBaseline, buffer);
    }
}
