#pragma once

#include "canvas.h"
#include "shot_curve.h"

/**
 * The finished-shot screen of the espresso mode: the shot's whole curve over
 * the full display, the last shot dotted behind it, its time under the time
 * axis and its yield on a dashed line at its height.
 */
namespace ShotGraph
{
    /** Draws shot, with last behind it, or without one if last is null. */
    void render(Canvas &canvas, const ShotCurve &shot, const ShotCurve *last);
}
