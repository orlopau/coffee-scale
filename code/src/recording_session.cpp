#include "recording_session.h"

using Status = RecorderScreens::Status;

RecordingSession::RecordingSession(Recorder &recorder, float gramsPerCount)
    : rec(recorder), gramsPerCount(gramsPerCount) {}

void RecordingSession::begin(unsigned long ms)
{
    if (!rec.isAllocated())
    {
        status = Status::NoMemory;
        return;
    }
    startRecording(ms);
}

void RecordingSession::startRecording(unsigned long ms)
{
    rec.start(ms);
    tared = false;
    status = Status::Recording;
}

void RecordingSession::sample(unsigned long ms, long raw)
{
    lastRaw = raw;
    if (status != Status::Recording)
    {
        return;
    }
    if (!tared)
    {
        tare = raw;
        tared = true;
    }
    rec.addSample(ms, raw);
    checkFull();
}

bool RecordingSession::click(unsigned long ms)
{
    if (status != Status::Recording)
    {
        return false;
    }
    rec.addClick(ms);
    checkFull();
    return true;
}

void RecordingSession::checkFull()
{
    if (rec.isFull())
    {
        status = Status::Full;
    }
}

bool RecordingSession::longPress()
{
    if (status == Status::Recording || status == Status::Full || status == Status::UploadFailed)
    {
        status = Status::Uploading;
        return true;
    }
    return false;
}

void RecordingSession::uploadFinished(bool ok, unsigned long ms)
{
    if (ok)
    {
        startRecording(ms);
    }
    else
    {
        status = Status::UploadFailed;
    }
}

RecorderScreens::State RecordingSession::state() const
{
    float grams = tared ? (lastRaw - tare) * gramsPerCount : 0;
    return {status, grams, rec.durationMs(), rec.remainingMs(), static_cast<unsigned int>(rec.clickCount())};
}
