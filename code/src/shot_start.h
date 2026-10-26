#pragma once

/**
 * Where a shot's start time came from. A pump start from a smart plug on the
 * machine comes later: it wins over the first drop minus the offset, and the
 * gap between the two is the preinfusion.
 */
enum class ShotStartSource
{
    /** The click on the espresso screen. */
    Click,
    /** The first drop the shot detector saw, minus the offset. */
    FirstDrop,
};
