#pragma once

class Mode
{
public:
    virtual ~Mode() {}
    virtual void update() = 0;
    virtual void enter() {};
    virtual bool canSwitchMode() = 0;
    virtual const char* getName() = 0;
};