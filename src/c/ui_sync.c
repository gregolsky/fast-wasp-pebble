// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0

#include "ui_sync.h"
#include "pins.h"

#include <pebble.h>

#define WAIT_MS 10000
#define WARN_MS 4000

static Window    *s_window;
static TextLayer *s_text;
static AppTimer  *s_timer;

static void leave(void *ctx) {
    (void)ctx;
    s_timer = NULL;
    window_stack_pop_all(true);
}

static void on_synced(void) {
    APP_LOG(APP_LOG_LEVEL_INFO, "sync-done");
    leave(NULL);
}

static void give_up(void *ctx) {
    (void)ctx;
    pins_when_synced(NULL);
    APP_LOG(APP_LOG_LEVEL_WARNING, "sync-failed");
    text_layer_set_font(s_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text(s_text, "Phone not reached.\nOpen FastWasp near your phone to update alerts.");
    s_timer = app_timer_register(WARN_MS, leave, NULL);
}

static void click_back(ClickRecognizerRef r, void *ctx) {
    (void)r; (void)ctx;
    leave(NULL);
}

static void click_config(void *ctx) {
    (void)ctx;
    window_single_click_subscribe(BUTTON_ID_BACK, click_back);
}

static void window_load(Window *win) {
    Layer *root  = window_get_root_layer(win);
    GRect  b     = layer_get_bounds(root);
    int    inset = PBL_IF_ROUND_ELSE(18, 6);
    s_text = text_layer_create(GRect(inset, b.size.h / 2 - 42, b.size.w - 2 * inset, 84));
    text_layer_set_background_color(s_text, GColorClear);
    text_layer_set_text_color(s_text, GColorWhite);
    text_layer_set_font(s_text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(s_text, GTextAlignmentCenter);
    text_layer_set_text(s_text, "Syncing alerts with phone\xe2\x80\xa6");
    layer_add_child(root, text_layer_get_layer(s_text));
    window_set_click_config_provider(win, click_config);

    APP_LOG(APP_LOG_LEVEL_INFO, "sync-wait");
    if (connection_service_peek_pebble_app_connection()) {
        pins_when_synced(on_synced);
        s_timer = app_timer_register(WAIT_MS, give_up, NULL);
    } else {
        give_up(NULL);
    }
}

static void window_unload(Window *win) {
    (void)win;
    pins_when_synced(NULL);
    if (s_timer) app_timer_cancel(s_timer);
    s_timer = NULL;
    text_layer_destroy(s_text);
    window_destroy(s_window);
    s_window = NULL;
}

void ui_sync_push(void) {
    s_window = window_create();
    window_set_background_color(s_window, GColorBlack);
    window_set_window_handlers(s_window, (WindowHandlers){
        .load   = window_load,
        .unload = window_unload,
    });
    window_stack_push(s_window, true);
}
