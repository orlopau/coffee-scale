#ifndef NATIVE

#pragma once

#include "network/setup_state.h"

/**
 * The setup page's HTTP server. Every path it doesn't know redirects to the
 * page, which makes phones open it right after joining.
 */
namespace SetupPortal
{
    /** Serves the page; the networks entered on it go to state. */
    bool start(SetupState &state);
    void stop();
}

#endif
