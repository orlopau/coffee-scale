#include "recording_session.h"

using Status = RecorderScreens::Status;

RecordingSession::RecordingSession(Recorder &recorder, float gramsPerCount, bool hasServer)
    : rec(recorder), gramsPerCount(gramsPerCount), hasServer(hasServer) {}

void RecordingSession::begin(size_t savedCount, bool storageReady)
{
    saved = savedCount;
    if (!rec.isAllocated())
    {
        status = Status::NoMemory;
        return;
    }
    if (!storageReady)
    {
        status = Status::NoStorage;
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
    if (status == Status::Idle || status == Status::SendFailed)
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
    if (status == Status::NoMemory || status == Status::NoStorage)
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

RecordingSession::Action RecordingSession::longPress()
{
    if (status == Status::Recording || status == Status::Full || status == Status::SaveFailed)
    {
        status = Status::Saving;
        return Action::Save;
    }
    if ((status == Status::Idle || status == Status::SendFailed) && hasServer && saved > 0)
    {
        status = Status::Sending;
        sendingTotal = saved;
        sendingIndex = 1;
        return Action::Send;
    }
    return Action::None;
}

void RecordingSession::saveFinished(bool ok)
{
    if (ok)
    {
        saved++;
        becomeIdle();
    }
    else
    {
        status = Status::SaveFailed;
    }
}

void RecordingSession::sent()
{
    if (saved > 0)
    {
        saved--;
    }
    sendingIndex++;
}

void RecordingSession::sendFinished(bool ok)
{
    if (ok)
    {
        becomeIdle();
    }
    else
    {
        status = Status::SendFailed;
    }
}

RecorderScreens::State RecordingSession::state() const
{
    float grams = tared ? (lastRaw - tareRaw) * gramsPerCount : 0;
    return {status, grams, rec.durationMs(), rec.remainingMs(), static_cast<unsigned int>(rec.clickCount()),
            saved, hasServer, sendingIndex, sendingTotal};
}
