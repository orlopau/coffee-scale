#pragma once

class LoadCell
{
public:
    virtual ~LoadCell() {}
    virtual void begin() = 0;
    virtual void update() = 0;
    virtual float getWeight() = 0;
    virtual bool isNewWeight() = 0;
    virtual void tare() = 0;
    virtual void setScale(float scale) = 0;
};