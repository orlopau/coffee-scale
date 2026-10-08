#pragma once

#include "canvas.h"

/**
 * A screen of the scale, e.g. the plain scale or espresso mode.
 *
 * update() is called on every loop and handles input and state. It must not draw.
 * render() draws the current state and is called by the ModeManager at a capped
 * frame rate, after clear() and before flush() of the canvas.
 *
 * Modes that still draw through the legacy Display:: functions inside update()
 * keep rendersToCanvas() false and are never asked to render.
 */
class Mode
{
public:
    virtual ~Mode() {}
    virtual void update() = 0;
    virtual void render(Canvas &canvas) {}
    virtual bool rendersToCanvas() { return false; }
    virtual void enter() {};
    virtual bool canSwitchMode() = 0;
    virtual const char* getName() = 0;
};
