#pragma once

class LoadCell
{
public:
    virtual ~LoadCell() {}
    virtual void begin() = 0;
    virtual long read() = 0;
    virtual bool isReady() = 0;
};

class WeightSensor
{
public:
    virtual void begin(){};
    virtual void update(){};
    virtual float getWeight() = 0;
    virtual bool isNewWeight() = 0;
    virtual void tare() = 0;
    virtual void setScale(float scale) = 0;
};

class DefaultWeightSensor : public WeightSensor
{
public:
    DefaultWeightSensor(LoadCell &loadCell);
    void begin() override;
    void update() override;
    float getWeight() override;
    bool isNewWeight() override;
    void tare() override;
    void setScale(float scale) override;

private:
    LoadCell &loadCell;
    float weight = 0;
    float scale = 1;
    long lastWeight = 0;
    long offset = 0;
    bool newWeight = false;
};