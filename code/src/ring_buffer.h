#pragma once

template <typename T, unsigned int N>
class RingBuffer
{
public:
    RingBuffer() : head(0), full(false){};
    void push(T value)
    {
        buffer[head] = value;
        if (head + 1 == N)
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
    T sum() const
    {
        T sum = 0;
        for (unsigned int i = 0; i < size(); i++)
        {
            sum += buffer[i];
        }
        return sum;
    };
    T average() const
    {
        return sum() / size();
    };
    unsigned int size() const
    {
        return full ? N : head;
    };
    unsigned int capacity() const
    {
        return N;
    };

private:
    T buffer[N];
    unsigned int head;
    bool full;
};
