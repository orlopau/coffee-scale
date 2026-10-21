#include "recording_session.h"

using Status = RecorderScreens::Status;

RecordingSession::RecordingSession(Recorder &recorder, float gramsPerCount)
    : rec(recorder), gramsPerCount(gramsPerCount) {}

void RecordingSession::begin()
{
    if (!rec.isAllocated())
    {
        status = Status::NoMemory;
        return;
    }
    becomeIdle();
}

void RecordingSession::becomeIdle()
{
    tared = false;
    status = Status::Idle;
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
    hasSample = true;
    if (!tared && (status == Status::Recording || status == Status::Idle))
    {
        tareRaw = raw;
        tared = true;
    }
    if (status != Status::Recording)
    {
        return;
    }
    rec.addSample(ms, raw);
    checkFull();
}

RecordingSession::Click RecordingSession::click(unsigned long ms)
{
    if (status == Status::Idle)
    {
        startRecording(ms);
        return Click::Started;
    }
    if (status != Status::Recording)
    {
        return Click::Ignored;
    }
    rec.addClick(ms);
    checkFull();
    return Click::Marked;
}

void RecordingSession::tare(unsigned long ms)
{
    if (status == Status::NoMemory)
    {
        return;
    }
    // before any sample, the next one is the tare
    tared = hasSample;
    tareRaw = lastRaw;
    if (status == Status::Recording)
    {
        rec.addTare(ms);
        checkFull();
    }
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

void RecordingSession::uploadFinished(bool ok)
{
    if (ok)
    {
        becomeIdle();
    }
    else
    {
        status = Status::UploadFailed;
    }
}

RecorderScreens::State RecordingSession::state() const
{
    float grams = tared ? (lastRaw - tareRaw) * gramsPerCount : 0;
    return {status, grams, rec.durationMs(), rec.remainingMs(), static_cast<unsigned int>(rec.clickCount())};
}
