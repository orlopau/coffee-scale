#pragma once

#include <vector>
#include "ring_buffer.h"

class AutoTare {
public:
    /**
     * Auto Tare tares the scale automatically, when it detects a known weight increment.
     * 
     * To achieve this, we maintain a list of "stable" weights. Once a stable weight changes to another stable weight,
     * we compare it to the list of known weight. If theres a match, shouldTare will return true, until its called once.
     */
    AutoTare(float tolerance, float maxStdDev, uint16_t bufferSize);
    bool shouldTare();
    void update(float rawWeight);
    std::vector<float> weights;
private:
    float tolerance;
    float maxStdDev;
    RingBuffer<float> buffer;
    bool isTare = false;

    float lastStableWeight = NAN;
};