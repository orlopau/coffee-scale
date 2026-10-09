#pragma once

unsigned long now();
void sleep_for(unsigned long millis);

#ifdef NATIVE
/**
 * Native builds run on a virtual clock: time only passes through sleep_for()
 * and set_now(), so timing-dependent code runs fast and reproducibly.
 */
void set_now(unsigned long millis);
#endif
