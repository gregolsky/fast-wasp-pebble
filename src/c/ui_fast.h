#pragma once

#include <pebble.h>

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
