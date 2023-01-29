#pragma once
#include <math.h>
#include <climits>

#include "ring_buffer.h"

namespace Regression
{
    struct Point
    {
        long x;
        float y;
    };

    struct Result
    {
        float m;
        float yIntercept;
    };

    class Approximator
    {
    public:
        Approximator(uint16_t bufferSize) : buffer(bufferSize) {}
        void reset();
        void addPoint(Point p);
        /**
         * Retrieves the approcimation of the given data points using least squares.
         * @return approximation
         */
        Result getLeastSquares();
        long getXAtY(float y);
        long getXAtY(float y, Result res);

    private:
        RingBuffer<Point> buffer;
    };
}