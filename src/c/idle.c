// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0

#include "idle.h"

#define IDLE_EXIT_MS 30000

static AppTimer *s_timer;

static void on_idle(void *ctx) {
    (void)ctx;
    s_timer = NULL;
    APP_LOG(APP_LOG_LEVEL_INFO, "idle-exit");
    window_stack_pop_all(true);
}

void idle_kick(void) {
    if (!s_timer || !app_timer_reschedule(s_timer, IDLE_EXIT_MS)) {
        s_timer = app_timer_register(IDLE_EXIT_MS, on_idle, NULL);
    }
}

void idle_menu_kick(MenuLayer *ml, MenuIndex new_idx, MenuIndex old_idx, void *ctx) {
    (void)ml; (void)new_idx; (void)old_idx; (void)ctx;
    idle_kick();
}

void idle_window_kick(Window *win) {
    (void)win;
    idle_kick();
}
