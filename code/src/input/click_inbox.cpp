#include "input/click_inbox.h"

void ClickInbox::update(uint32_t now)
{
    click = ClickType::NONE;
    ClickEvent event;
    while (source.next(event))
    {
        // signed, so a click stamped just after now counts as fresh, and the
        // wrap of millis() does no harm
        int32_t age = static_cast<int32_t>(now - event.ms);
        if (age <= static_cast<int32_t>(MAX_AGE_MS))
        {
            click = event.type;
            return;
        }
    }
}

ClickType ClickInbox::consume()
{
    ClickType result = click;
    click = ClickType::NONE;
    return result;
}

void ClickInbox::discard()
{
    click = ClickType::NONE;
    ClickEvent event;
    while (source.next(event))
    {
    }
}
