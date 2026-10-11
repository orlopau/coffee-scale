#include "weighing/flow_estimator.h"

FlowEstimator::FlowEstimator(uint32_t windowMs) : windowMs(windowMs), points(CAPACITY) {}

float FlowEstimator::add(uint32_t ms, float grams)
{
    points.push({ms, grams});

    double sumX = 0, sumY = 0, sumXX = 0, sumXY = 0;
    unsigned int n = 0;
    for (unsigned int i = 0; i < points.size(); i++)
    {
        Point p = points.getRelative(-static_cast<int>(i));
        // unsigned, so samples from after the newest one are too old as well
        uint32_t age = ms - p.ms;
        if (age > windowMs)
        {
            continue;
        }
        double x = -(age / 1000.0);
        double y = p.grams - grams;
        sumX += x;
        sumY += y;
        sumXX += x * x;
        sumXY += x * y;
        n++;
    }

    double denominator = n * sumXX - sumX * sumX;
    if (n < 2 || denominator <= 0)
    {
        return 0;
    }
    return static_cast<float>((n * sumXY - sumX * sumY) / denominator);
}

void FlowEstimator::reset() { points.clear(); }
