#include "regression.h"

namespace Regression
{    
    void Approximator::reset() { buffer.clear(); }

    void Approximator::addPoint(Point p) { buffer.push(p); }

    Result Approximator::getLeastSquares()
    {
        float sumX = 0;
        float sumY = 0;
        float sumX2 = 0;
        float sumXY = 0;

        int bufferSize = buffer.size();
        for (int i = 0; i < bufferSize; i++)
        {
            Point p = buffer.get(i);
            sumX += p.x;
            sumY += p.y;
            sumX2 += p.x * p.x;
            sumXY += p.x * p.y;
        }

        float m = (bufferSize * sumXY - sumX * sumY) / (bufferSize * sumX2 - sumX * sumX);
        float yIntercept = (sumY - m * sumX) / bufferSize;

        return {
            m,
            yIntercept,
        };
    }

    long Approximator::getXAtY(float y) { return getXAtY(y, getLeastSquares()); }

    long Approximator::getXAtY(float y, Result res)
    {
        if (isnan(res.m))
        {
            return LONG_MAX;
        }

        return (y - res.yIntercept) / res.m;
    }
}
