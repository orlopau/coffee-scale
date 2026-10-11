#pragma once

#include "recorder.h"
#include "ui/recorder_screens.h"

/**
 * What the recorder does with the scale's inputs: a click starts a recording,
 * samples and further clicks go into the Recorder while it records, and a long
 * press asks for an upload. After a successful upload it waits for the click
 * that starts the next recording. The device only
 * passes the inputs in and does the upload, so this logic runs in native
 * tests.
 */
class RecordingSession
{
public:
    RecordingSession(Recorder &recorder, float gramsPerCount);

    enum class Click
    {
        Ignored,
        Started,
        Marked,
    };

    /** Waits for the click that starts recording, or shows that there is no memory for it. */
    void begin();
    /**
     * Recorded only while recording. The first sample of a recording, or of
     * waiting for one, is the tare of the shown weight.
     */
    void sample(unsigned long ms, long raw);
    /** Starts a recording at ms while waiting, places a marker while recording. The caller beeps unless ignored. */
    Click click(unsigned long ms);
    /** Returns true if the caller must upload the recording now. */
    bool longPress();
    /** On success it waits for the next recording, otherwise the recording is kept for a retry. */
    void uploadFinished(bool ok);

    RecorderScreens::State state() const;
    const Recorder &recorder() const { return rec; }

private:
    void becomeIdle();
    void startRecording(unsigned long ms);
    void checkFull();

    Recorder &rec;
    float gramsPerCount;
    RecorderScreens::Status status = RecorderScreens::Status::Idle;
    bool tared = false;
    long tare = 0;
    long lastRaw = 0;
};
