#ifndef NATIVE

#pragma once

#include <stddef.h>

/** Downloads a firmware into the other app partition and boots it next. */
namespace FirmwareDownload
{
    enum class Result
    {
        Installed,
        /** The server answered 304: it has the running firmware. */
        NoUpdate,
        Failed,
    };

    /** Called while downloading; total is 0 if the server doesn't say. */
    typedef void (*Progress)(size_t done, size_t total);

    /**
     * Downloads url, following redirects, and makes it the firmware to boot.
     * https:// checks the server's certificate against the CA bundle,
     * http:// is for the dev server. runningMd5 goes along as
     * x-ESP32-sketch-md5, like HTTPUpdate sent it. On failure, logged, the
     * running firmware stays the one to boot.
     */
    Result run(const char *url, Progress progress, const char *runningMd5);
}

#endif
