#ifndef NATIVE

#pragma once

#include <stddef.h>

/** Finds the latest release's version without downloading anything. */
namespace ReleaseCheck
{
    /**
     * Sends HEAD requests to url and follows its redirects, up to 5, until
     * one points at a release's download (".../download/firmware-<version>/..."),
     * and writes that version into version. Following them also covers a
     * renamed or moved repository, whose old address redirects to the new
     * one first. False, and logged, if none points at a release, e.g. a guest
     * network's login page. https:// checks the server's certificate against
     * the CA bundle, like FirmwareDownload.
     */
    bool latestVersion(const char *url, char *version, size_t size);
}

#endif
