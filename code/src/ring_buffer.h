#pragma once
#include <stdint.h>
#include <cmath>

template <typename T>
class RingBuffer
{
public:
    ~RingBuffer()
    {
        delete[] buffer;
    };
    RingBuffer(uint8_t bufferSize) : head(0), full(false), bufferSize(bufferSize)
    {
        buffer = new T[bufferSize];
    };
    void push(T value)
    {
        buffer[head] = value;
        if (head + 1 == bufferSize)
        {
            head = 0;
            full = true;
        }
        else
        {
            head++;
        }
    };
    T get(unsigned int index) const
    {
        return buffer[index];
    };
    T getRelative(int relative) const
    {
        int index = head + relative - 1;
        if (index < 0)
        {
            index += bufferSize;
        }
        return buffer[index];
    };
    T sumLast(unsigned int count) const
    {
        T sum = 0;
        for (int i = 0; i < count; i++)
        {
            sum += getRelative(-i);
        }
        return sum;
    };
    T averageLast(unsigned int count) const
    {
        return sumLast(count) / (float)count;
    };
    unsigned int size() const
    {
        return full ? bufferSize : head;
    };
    unsigned int capacity() const
    {
        return bufferSize;
    };
    void clear()
    {
        head = 0;
        full = false;
    };
    float varianceLast(unsigned int count) const
    {
        float mean = averageLast(count);
        float variance = 0;
        for (int i = 0; i < count; i++)
        {
            float diff = getRelative(-i) - mean;
            variance += diff * diff;
        }
        return variance / count;
    };
    float standardDeviationLast(unsigned int count) const { return std::sqrt(varianceLast(count)); };

private:
    T *buffer;
    unsigned int bufferSize;
    unsigned int head;
    bool full;
};
