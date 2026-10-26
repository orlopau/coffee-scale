#include <unity.h>

#include <string>

#include "recorder.h"
#include "recording_session.h"

using Status = RecorderScreens::Status;

static Recorder *recorder;
static RecordingSession *session;

void setUp(void)
{
    recorder = new Recorder();
    session = new RecordingSession(*recorder, 0.01f, true);
}

void tearDown(void)
{
    delete session;
    delete recorder;
}

/** Begins and starts recording at ms 0. */
static void startRecording(RecordingSession &s)
{
    s.begin(0, true);
    TEST_ASSERT_EQUAL(RecordingSession::Click::Started, s.click(0));
}

void test_waits_for_click_to_start(void)
{
    session->begin(0, true);
    TEST_ASSERT_EQUAL(Status::Idle, session->state().status);
    session->sample(0, 1000);
    session->sample(100, 1300);
    TEST_ASSERT_EQUAL(0, recorder->sampleCount());
    TEST_ASSERT_EQUAL(RecordingSession::Action::None, session->longPress());

    TEST_ASSERT_EQUAL(RecordingSession::Click::Started, session->click(200));
    TEST_ASSERT_EQUAL(Status::Recording, session->state().status);
    TEST_ASSERT_EQUAL(0, recorder->clickCount());
    session->sample(200, 1500);
    TEST_ASSERT_EQUAL(1, recorder->sampleCount());
    TEST_ASSERT_EQUAL_FLOAT(0, session->state().grams);
}

void test_weight_shown_while_idle(void)
{
    session->begin(0, true);
    session->sample(0, 1000);
    session->sample(100, 1300);
    TEST_ASSERT_EQUAL_FLOAT(3.0f, session->state().grams);
}

void test_records_samples_and_clicks(void)
{
    startRecording(*session);
    session->sample(0, 1000);
    session->sample(100, 1400);

    TEST_ASSERT_EQUAL_FLOAT(4.0f, session->state().grams);
    TEST_ASSERT_EQUAL(Status::Recording, session->state().status);
    TEST_ASSERT_EQUAL(100, session->state().recordedMs);

    TEST_ASSERT_EQUAL(RecordingSession::Click::Marked, session->click(150));
    TEST_ASSERT_EQUAL(1, session->state().clicks);
    TEST_ASSERT_EQUAL(2, recorder->sampleCount());
}

void test_long_press_saves_and_waits(void)
{
    startRecording(*session);
    session->sample(0, 1000);

    TEST_ASSERT_EQUAL(RecordingSession::Action::Save, session->longPress());
    TEST_ASSERT_EQUAL(Status::Saving, session->state().status);
    session->sample(100, 1100);
    TEST_ASSERT_EQUAL(1, recorder->sampleCount());

    session->saveFinished(true);
    TEST_ASSERT_EQUAL(Status::Idle, session->state().status);
    session->sample(500, 2000);
    TEST_ASSERT_EQUAL(1, recorder->sampleCount()); // the saved one, not recording

    TEST_ASSERT_EQUAL(RecordingSession::Click::Started, session->click(600));
    TEST_ASSERT_EQUAL(0, recorder->sampleCount());
    session->sample(600, 2500);
    TEST_ASSERT_EQUAL_FLOAT(0, session->state().grams);
    TEST_ASSERT_EQUAL(1, recorder->sampleCount());
}

void test_failed_save_keeps_recording(void)
{
    startRecording(*session);
    session->sample(0, 1000);
    session->click(50);
    session->longPress();

    session->saveFinished(false);
    TEST_ASSERT_EQUAL(Status::SaveFailed, session->state().status);
    TEST_ASSERT_EQUAL(1, recorder->sampleCount());
    TEST_ASSERT_EQUAL(1, recorder->clickCount());

    session->sample(600, 1100);
    TEST_ASSERT_EQUAL(1, recorder->sampleCount());

    TEST_ASSERT_EQUAL(RecordingSession::Action::Save, session->longPress());
    TEST_ASSERT_EQUAL(1, recorder->sampleCount());
    session->saveFinished(true);
    TEST_ASSERT_EQUAL(Status::Idle, session->state().status);
}

void test_click_ignored_while_not_recording(void)
{
    Recorder small(2);
    RecordingSession smallSession(small, 0.01f, true);
    startRecording(smallSession);
    smallSession.sample(0, 1000);

    smallSession.longPress();
    TEST_ASSERT_EQUAL(RecordingSession::Click::Ignored, smallSession.click(10)); // saving
    smallSession.saveFinished(false);
    TEST_ASSERT_EQUAL(RecordingSession::Click::Ignored, smallSession.click(30)); // save failed
    TEST_ASSERT_EQUAL(0, small.clickCount());

    smallSession.longPress();
    smallSession.saveFinished(true);
    smallSession.click(100);
    smallSession.sample(100, 1000);
    smallSession.sample(200, 1000);
    TEST_ASSERT_EQUAL(Status::Full, smallSession.state().status);
    TEST_ASSERT_EQUAL(RecordingSession::Click::Ignored, smallSession.click(250)); // full
    TEST_ASSERT_EQUAL(0, smallSession.state().clicks);
}

void test_full(void)
{
    Recorder small(2);
    RecordingSession smallSession(small, 0.01f, true);
    startRecording(smallSession);
    smallSession.sample(0, 1000);
    smallSession.sample(100, 1010);

    TEST_ASSERT_EQUAL(Status::Full, smallSession.state().status);
    smallSession.sample(200, 1020);
    TEST_ASSERT_EQUAL(2, small.sampleCount());
    TEST_ASSERT_EQUAL(RecordingSession::Action::Save, smallSession.longPress());
}

void test_full_by_click(void)
{
    Recorder small(2);
    RecordingSession smallSession(small, 0.01f, true);
    startRecording(smallSession);
    smallSession.sample(0, 1000);
    TEST_ASSERT_EQUAL(RecordingSession::Click::Marked, smallSession.click(50));

    TEST_ASSERT_EQUAL(Status::Full, smallSession.state().status);
}

void test_weight_shown_while_not_recording(void)
{
    startRecording(*session);
    session->sample(0, 1000);
    session->longPress();
    session->saveFinished(false);

    session->sample(200, 1500); // not recorded, but the scale still shows what is on it
    TEST_ASSERT_EQUAL_FLOAT(5.0f, session->state().grams);
}

void test_no_memory(void)
{
    Recorder none(0);
    RecordingSession noneSession(none, 0.01f, true);
    noneSession.begin(0, true);

    TEST_ASSERT_EQUAL(Status::NoMemory, noneSession.state().status);
    TEST_ASSERT_EQUAL(RecordingSession::Action::None, noneSession.longPress());
    TEST_ASSERT_EQUAL(RecordingSession::Click::Ignored, noneSession.click(10));
    noneSession.sample(20, 1000);
    TEST_ASSERT_EQUAL(Status::NoMemory, noneSession.state().status);
}

// Samples are timestamped when measured and taken a little later, so one
// measured just before the start click can arrive after it.
void test_sample_from_before_the_start_is_not_recorded(void)
{
    session->begin(0, true);
    session->click(1000);
    session->sample(995, 1000);

    TEST_ASSERT_EQUAL(0, recorder->sampleCount());
    TEST_ASSERT_EQUAL(Status::Recording, session->state().status);
}

void test_tare_while_idle(void)
{
    session->begin(0, true);
    session->sample(0, 1000);
    session->sample(100, 1300);
    session->tare(150);

    TEST_ASSERT_EQUAL_FLOAT(0, session->state().grams);
    session->sample(200, 1400);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, session->state().grams);
    TEST_ASSERT_EQUAL(Status::Idle, session->state().status);
    TEST_ASSERT_EQUAL(0, recorder->sampleCount());
}

void test_tare_while_recording_is_recorded(void)
{
    startRecording(*session);
    session->sample(0, 1000);
    session->sample(100, 16000); // a cup
    session->tare(150);

    TEST_ASSERT_EQUAL_FLOAT(0, session->state().grams);
    session->sample(200, 16500);
    TEST_ASSERT_EQUAL_FLOAT(5.0f, session->state().grams);
    TEST_ASSERT_EQUAL(3, recorder->sampleCount());
    TEST_ASSERT_EQUAL(0, session->state().clicks);

    std::string csv;
    recorder->writeCsv("v1.9.0", 0.01f, [&](const char *data, size_t length) { csv.append(data, length); });
    TEST_ASSERT_NOT_EQUAL(std::string::npos, csv.find("150,tare,\n"));
}

void test_tare_not_recorded_while_not_recording(void)
{
    startRecording(*session);
    session->sample(0, 1000);
    session->longPress();
    session->saveFinished(false);

    session->sample(100, 1500);
    session->tare(150);
    TEST_ASSERT_EQUAL_FLOAT(0, session->state().grams);

    std::string csv;
    recorder->writeCsv("v1.9.0", 0.01f, [&](const char *data, size_t length) { csv.append(data, length); });
    TEST_ASSERT_EQUAL(std::string::npos, csv.find("tare"));
}

void test_tare_before_any_sample(void)
{
    session->begin(0, true);
    session->tare(0);
    session->sample(100, 1300);

    TEST_ASSERT_EQUAL_FLOAT(0, session->state().grams);
}

void test_tare_fills_up(void)
{
    Recorder small(2);
    RecordingSession smallSession(small, 0.01f, true);
    startRecording(smallSession);
    smallSession.sample(0, 1000);
    smallSession.tare(50);

    TEST_ASSERT_EQUAL(Status::Full, smallSession.state().status);
}

using Action = RecordingSession::Action;

void test_long_press_saves_the_recording(void)
{
    startRecording(*session);
    session->sample(0, 1000);
    TEST_ASSERT_EQUAL(Action::Save, session->longPress());
    TEST_ASSERT_EQUAL(Status::Saving, session->state().status);
    session->saveFinished(true);
    TEST_ASSERT_EQUAL(Status::Idle, session->state().status);
    TEST_ASSERT_EQUAL(1, session->state().saved);
}

void test_save_failure_keeps_the_recording(void)
{
    startRecording(*session);
    session->sample(0, 1000);
    session->longPress();
    session->saveFinished(false);
    TEST_ASSERT_EQUAL(Status::SaveFailed, session->state().status);
    TEST_ASSERT_EQUAL(1, recorder->sampleCount());
    TEST_ASSERT_EQUAL(Action::Save, session->longPress());
}

void test_long_press_when_idle_sends(void)
{
    session->begin(3, true);
    TEST_ASSERT_EQUAL(Action::Send, session->longPress());
    TEST_ASSERT_EQUAL(Status::Sending, session->state().status);
    TEST_ASSERT_EQUAL(1, session->state().sendingIndex);
    TEST_ASSERT_EQUAL(3, session->state().sendingTotal);
    session->sent();
    session->sent();
    TEST_ASSERT_EQUAL(1, session->state().saved);
    TEST_ASSERT_EQUAL(3, session->state().sendingIndex);
    session->sent();
    session->sendFinished(true);
    TEST_ASSERT_EQUAL(Status::Idle, session->state().status);
    TEST_ASSERT_EQUAL(0, session->state().saved);
}

void test_send_failure_keeps_the_rest(void)
{
    session->begin(5, true);
    TEST_ASSERT_EQUAL(Action::Send, session->longPress());
    session->sent();
    session->sent();
    session->sendFinished(false);
    TEST_ASSERT_EQUAL(Status::SendFailed, session->state().status);
    TEST_ASSERT_EQUAL(3, session->state().saved);
    TEST_ASSERT_EQUAL(Action::Send, session->longPress());
    TEST_ASSERT_EQUAL(3, session->state().sendingTotal);
    session->sendFinished(false);
    TEST_ASSERT_EQUAL(RecordingSession::Click::Started, session->click(0));
}

void test_nothing_to_send_without_server(void)
{
    Recorder r;
    RecordingSession s(r, 0.01f, false);
    s.begin(2, true);
    TEST_ASSERT_EQUAL(Action::None, s.longPress());
    TEST_ASSERT_EQUAL(Status::Idle, s.state().status);
    TEST_ASSERT_FALSE(s.state().hasServer);
}

void test_nothing_to_send_when_empty(void)
{
    session->begin(0, true);
    TEST_ASSERT_EQUAL(Action::None, session->longPress());
}

void test_long_press_while_busy_is_ignored(void)
{
    startRecording(*session);
    session->longPress();
    TEST_ASSERT_EQUAL(Action::None, session->longPress());
    TEST_ASSERT_EQUAL(RecordingSession::Click::Ignored, session->click(10));
    TEST_ASSERT_EQUAL(Status::Saving, session->state().status);

    session->begin(2, true);
    session->longPress();
    TEST_ASSERT_EQUAL(Action::None, session->longPress());
    TEST_ASSERT_EQUAL(RecordingSession::Click::Ignored, session->click(10));
    TEST_ASSERT_EQUAL(Status::Sending, session->state().status);
}

void test_no_storage(void)
{
    session->begin(0, false);
    TEST_ASSERT_EQUAL(Status::NoStorage, session->state().status);
    TEST_ASSERT_EQUAL(RecordingSession::Click::Ignored, session->click(0));
    TEST_ASSERT_EQUAL(Action::None, session->longPress());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_waits_for_click_to_start);
    RUN_TEST(test_weight_shown_while_idle);
    RUN_TEST(test_records_samples_and_clicks);
    RUN_TEST(test_long_press_saves_and_waits);
    RUN_TEST(test_failed_save_keeps_recording);
    RUN_TEST(test_click_ignored_while_not_recording);
    RUN_TEST(test_full);
    RUN_TEST(test_full_by_click);
    RUN_TEST(test_weight_shown_while_not_recording);
    RUN_TEST(test_no_memory);
    RUN_TEST(test_sample_from_before_the_start_is_not_recorded);
    RUN_TEST(test_tare_while_idle);
    RUN_TEST(test_tare_while_recording_is_recorded);
    RUN_TEST(test_tare_not_recorded_while_not_recording);
    RUN_TEST(test_tare_before_any_sample);
    RUN_TEST(test_tare_fills_up);
    RUN_TEST(test_long_press_saves_the_recording);
    RUN_TEST(test_save_failure_keeps_the_recording);
    RUN_TEST(test_long_press_when_idle_sends);
    RUN_TEST(test_send_failure_keeps_the_rest);
    RUN_TEST(test_nothing_to_send_without_server);
    RUN_TEST(test_nothing_to_send_when_empty);
    RUN_TEST(test_long_press_while_busy_is_ignored);
    RUN_TEST(test_no_storage);
    return UNITY_END();
}
