#pragma once

class Stopwatch
{
    public:
        void start();
        /** Starts running as if started at startMs, a now() time in the past. */
        void startAt(unsigned long startMs);
        /** Does nothing if it isn't running. */
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