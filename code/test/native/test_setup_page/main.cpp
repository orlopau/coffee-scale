#include <string.h>
#include <unity.h>

#include "data/setup_page.h"

void setUp(void) {}

void tearDown(void) {}

// the page's own display:block on labels would otherwise beat the browser's [hidden]
void test_hidden_elements_stay_hidden(void)
{
    TEST_ASSERT_NOT_NULL(strstr(SETUP_PAGE, "[hidden]{display:none!important}"));
}

// a page the phone reloads, e.g. after the channel switch, picks up a connection still being tried
void test_the_page_asks_for_the_status_when_it_loads(void)
{
    TEST_ASSERT_NOT_NULL(strstr(SETUP_PAGE, "poll();</script>"));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_hidden_elements_stay_hidden);
    RUN_TEST(test_the_page_asks_for_the_status_when_it_loads);
    return UNITY_END();
}
