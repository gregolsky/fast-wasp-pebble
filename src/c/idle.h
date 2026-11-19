// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <pebble.h>

// Back to the watchface after IDLE_EXIT_MS with no button pressed.
// Call idle_kick() on every button press; it also starts the countdown.
void idle_kick(void);
// Window .appear handler: a screen opening or coming back (after BACK) counts as use.
void idle_window_kick(Window *win);
// MenuLayer selection_changed callback that calls idle_kick().
void idle_menu_kick(MenuLayer *ml, MenuIndex new_idx, MenuIndex old_idx, void *ctx);
