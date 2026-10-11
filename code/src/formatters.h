#pragma once

/**
 * The weight with its unit, e.g. "12.30g". A weight that rounds to zero shows
 * as 0, so noise just below zero doesn't show as "-0.00g".
 */
char *formatWeight(float grams, int decimals = 2);
char *formatTime(unsigned long millis);
