#if !defined(NATIVE) && defined(NET_LOG)

#pragma once

/**
 * The log firmware's log: every ESP_LOG line also goes to the dev server as
 * UDP, so the log can be read without a serial cable.
 */
namespace NetLog
{
    /** Copies every log line to the dev server from now on, see the spec. Call first in setup(),
     *  in the loop's task: it also calls WifiStation::begin(). */
    void begin();
}

#endif
