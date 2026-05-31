#pragma once

#ifdef FAST_PEBBLE_HOST_BUILD
#include <stdint.h>
extern int32_t g_now;
static inline int32_t time_now(void) { return g_now; }
#else
#include <pebble.h>
static inline int32_t time_now(void) { return (int32_t)time(NULL); }
#endif
