#include <deque>
#include <unity.h>

#include "input/click_inbox.h"

class FakeClicks : public ClickSource
{
public:
    std::deque<ClickEvent> events;

    bool next(ClickEvent &event) override
    {
        if (events.empty())
        {
            return false;
        }
        event = events.front();
        events.pop_front();
        return true;
    }
};

FakeClicks *clicks;
ClickInbox *inbox;

void setUp(void)
{
    clicks = new FakeClicks();
    inbox = new ClickInbox(*clicks);
}

void tearDown(void)
{
    delete inbox;
    delete clicks;
}

void test_no_click_without_events(void)
{
    inbox->update(1000);
    TEST_ASSERT_EQUAL(ClickType::NONE, inbox->get());
}

void test_click_stays_until_the_next_update(void)
{
    clicks->events.push_back({1000, ClickType::SINGLE});
    inbox->update(1010);
    TEST_ASSERT_EQUAL(ClickType::SINGLE, inbox->get());
    TEST_ASSERT_EQUAL(ClickType::SINGLE, inbox->get());

    inbox->update(1020);
    TEST_ASSERT_EQUAL(ClickType::NONE, inbox->get());
}

void test_consume_clears_the_click(void)
{
    clicks->events.push_back({1000, ClickType::LONG});
    inbox->update(1000);
    TEST_ASSERT_EQUAL(ClickType::LONG, inbox->consume());
    TEST_ASSERT_EQUAL(ClickType::NONE, inbox->get());
    TEST_ASSERT_EQUAL(ClickType::NONE, inbox->consume());
}

void test_one_click_per_update(void)
{
    clicks->events.push_back({1000, ClickType::SINGLE});
    clicks->events.push_back({1100, ClickType::LONG});
    inbox->update(1200);
    TEST_ASSERT_EQUAL(ClickType::SINGLE, inbox->get());
    inbox->update(1210);
    TEST_ASSERT_EQUAL(ClickType::LONG, inbox->get());
    inbox->update(1220);
    TEST_ASSERT_EQUAL(ClickType::NONE, inbox->get());
}

void test_drops_clicks_older_than_a_second(void)
{
    clicks->events.push_back({1000, ClickType::SINGLE});
    clicks->events.push_back({1999, ClickType::LONG});
    inbox->update(2001);
    // the first is 1001 ms old, the second 2 ms
    TEST_ASSERT_EQUAL(ClickType::LONG, inbox->get());
}

void test_keeps_a_click_exactly_a_second_old(void)
{
    clicks->events.push_back({1000, ClickType::SINGLE});
    inbox->update(2000);
    TEST_ASSERT_EQUAL(ClickType::SINGLE, inbox->get());
}

void test_keeps_a_click_stamped_after_now(void)
{
    // the button task pushed it after the loop took its time
    clicks->events.push_back({1003, ClickType::SINGLE});
    inbox->update(1000);
    TEST_ASSERT_EQUAL(ClickType::SINGLE, inbox->get());
}

void test_keeps_a_click_across_the_millis_wrap(void)
{
    clicks->events.push_back({0xFFFFFF00u, ClickType::SINGLE});
    inbox->update(0x100u);
    TEST_ASSERT_EQUAL(ClickType::SINGLE, inbox->get());
}

void test_discard_empties_the_queue(void)
{
    clicks->events.push_back({1000, ClickType::LONG});
    clicks->events.push_back({1010, ClickType::SINGLE});
    inbox->update(1020);
    inbox->discard();
    TEST_ASSERT_EQUAL(ClickType::NONE, inbox->get());
    TEST_ASSERT_TRUE(clicks->events.empty());
    inbox->update(1030);
    TEST_ASSERT_EQUAL(ClickType::NONE, inbox->get());
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_no_click_without_events);
    RUN_TEST(test_click_stays_until_the_next_update);
    RUN_TEST(test_consume_clears_the_click);
    RUN_TEST(test_one_click_per_update);
    RUN_TEST(test_drops_clicks_older_than_a_second);
    RUN_TEST(test_keeps_a_click_exactly_a_second_old);
    RUN_TEST(test_keeps_a_click_stamped_after_now);
    RUN_TEST(test_keeps_a_click_across_the_millis_wrap);
    RUN_TEST(test_discard_empties_the_queue);
    return UNITY_END();
}
