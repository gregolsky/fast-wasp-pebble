#pragma once

#include <pebble.h>
#include "notify.h"

typedef enum {
    FAST_STATE_READY,
    FAST_STATE_ACTIVE,
    FAST_STATE_EATING,
    FAST_STATE_OMAD,
} FastUIState;

// Push (or replace) the main fast window.
void ui_fast_push(void);

// Refresh the displayed state from storage (called after state changes).
void ui_fast_refresh(void);

// Show a temporary banner in the hint row (auto-clears after ~5 ticks).
void ui_fast_show_banner(NotifyEvent ev);
