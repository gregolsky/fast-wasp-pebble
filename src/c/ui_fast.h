#pragma once

#include <pebble.h>

// Push (or replace) the main fast window.
void ui_fast_push(void);

// Refresh the displayed state from storage (called after state changes).
void ui_fast_refresh(void);
