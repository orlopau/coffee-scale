#pragma once

#include "recorder.h"
#include "ui/recorder_screens.h"

/**
 * What the recorder does with the scale's inputs: samples and clicks go into
 * the Recorder while it records, a long press asks for an upload, and the
 * upload's result decides whether a new recording starts. The device only
 * passes the inputs in and does the upload, so this logic runs in native
 * tests.
 */
class RecordingSession
{
public:
    RecordingSession(Recorder &recorder, float gramsPerCount);

    /** Starts recording, or shows that there is no memory for it. */
    void begin(unsigned long ms);
    /** Recorded only while recording. The first sample of a recording is the tare of the shown weight. */
    void sample(unsigned long ms, long raw);
    /** Places a marker while recording. Returns true if one was placed, the caller beeps. */
    bool click(unsigned long ms);
    /** Returns true if the caller must upload the recording now. */
    bool longPress();
    /** On success a new recording starts at ms, otherwise the recording is kept for a retry. */
    void uploadFinished(bool ok, unsigned long ms);

    RecorderScreens::State state() const;
    const Recorder &recorder() const { return rec; }

private:
    void startRecording(unsigned long ms);
    void checkFull();

    Recorder &rec;
    float gramsPerCount;
    RecorderScreens::Status status = RecorderScreens::Status::Recording;
    bool tared = false;
    long tare = 0;
    long lastRaw = 0;
};
