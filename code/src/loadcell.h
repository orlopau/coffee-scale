#pragma once

#include "stdint.h"
#include "ring_buffer.h"

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
    virtual ~WeightSensor(){};
    virtual void begin(){};
    virtual void update(){};
    virtual float getWeight() = 0;
    virtual bool isNewWeight() = 0;
    virtual void tare() = 0;
    virtual void setScale(float scale) = 0;
    /**
     * @brief Set auto averaging for the sensor.
     *
     * Auto averaging works by increasing the samples used to calculate the
     * weight, if the weight changes by less than the deltaPerSecondChange parameter.
     *
     * @param deltaChange The delta change in units per second under which auto
     * averaging is enabled. Set to 0 to disable auto averaging.
     * @param samples The number of samples to use when averaging is activated
     */
    virtual void setAutoAveraging(unsigned long deltaChange, uint8_t samples){};
};

class DefaultWeightSensor : public WeightSensor
{
public:
    ~DefaultWeightSensor() override;
    DefaultWeightSensor(LoadCell &loadCell);
    void begin() override;
    void update() override;
    float getWeight() override;
    bool isNewWeight() override;
    void tare() override;
    void setScale(float scale) override;
    void setAutoAveraging(unsigned long deltaChange, uint8_t samples) override;
    long getRawWeight();

private:
    void updateAveraging(long lastWeight, long newWeight);

    LoadCell &loadCell;
    float weight = 0;
    float scale = 1;
    long offset = 0;
    bool newWeight = false;

    RingBuffer<long> *ringBuffer = nullptr;
    unsigned long deltaPerSChange = 0;
    uint8_t samples = 0;
    unsigned long lastWeightTime = 0;
    unsigned int numAveragingActivations = 0;
};