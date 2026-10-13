#ifndef NATIVE

#pragma once

/**
 * Automatic updates: every few starts the scale checks in the background
 * whether a newer firmware is out, and asks about it at the next start. The
 * decisions are in auto_update_policy.h.
 */
namespace AutoUpdate
{
    /**
     * Counts the start and, if a check is due and a network is saved, checks
     * for a newer release in a task of its own, online through a
     * WifiStation::Online meanwhile. Call once the modes run.
     */
    void begin();

    /**
     * At start, before the modes: after a rollback, remembers the version an
     * automatic update installed, so it isn't offered again. Then asks about
     * a newer version a check found, "Yes" installs it and restarts. Blocks
     * and draws its own screens, like the Wi-Fi question.
     */
    void askAtStart(bool rolledBack);

    /** The running firmware works, or one is installed by hand: no automatic update is pending. */
    void clearInstalling();
}

#endif
