#pragma once

#include "mocks.h"
#include "stopwatch.h"

static Stopwatch *stopwatch;
static MockLoadCell *loadCell;
static MockButtons *buttons;
static MockDisplay *display;

static void (*currSetUp)();
static void (*currTearDown)();