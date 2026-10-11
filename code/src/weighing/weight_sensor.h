#pragma once

#include "stdint.h"
#include "ring_buffer.h"

class WeightSensor
{
public:
    virtual ~WeightSensor(){};
    virtual void begin(){};
    virtual void update(){};
    /**
     * @brief Gets the scaled weight in grams. Could be averaged depending on the set auto averaging mode.
     * 
     * @return weight in grams
     */
    virtual float getWeight() = 0;
    /**
     * @brief Gets the latest scaled weight in grams. This is not averaged.
     * 
     * @return weight in grams
     */
    virtual float getLastWeight() = 0;
    virtual float getLastUntaredWeight() = 0;
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
    DefaultWeightSensor();
    void begin() override;
    void update() override;
    float getWeight() override;
    float getLastWeight() override;
    float getLastUntaredWeight() override;
    bool isNewWeight() override;
    void tare() override;
    void setScale(float scale) override;
    void setAutoAveraging(unsigned long deltaChange, uint8_t samples) override;
    long getRawWeight();

private:
    void updateAveraging(long lastWeight, long newWeight);

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