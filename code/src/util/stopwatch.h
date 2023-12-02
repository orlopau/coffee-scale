#pragma once

class Stopwatch
{
    public:
        void start();
        void stop();
        void toggle();
        void reset();
        unsigned long getTime();
        bool isRunning();
    private:
        unsigned long startTime = 0;
        unsigned long stopTime = 0;
        bool running = false;
};