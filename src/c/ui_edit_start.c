// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0

#include "ui_edit_start.h"
#include "fasting.h"
#include "storage.h"

#include <pebble.h>
#include <stdio.h>

static Window    *s_window;
static TextLayer *s_header;
static TextLayer *s_offset_label;
static TextLayer *s_error_label;
static int32_t    s_offset_minutes;

static void update_display(void) {
    char buf[16];
    if (s_offset_minutes == 0) {
        snprintf(buf, sizeof(buf), "0 min");
    } else {
        snprintf(buf, sizeof(buf), "%+d min", (int)s_offset_minutes);
    }
    text_layer_set_text(s_offset_label, buf);
    text_layer_set_text(s_error_label, "");
}

static void click_up(ClickRecognizerRef r, void *ctx) {
    (void)r; (void)ctx;
    s_offset_minutes += 15;
    update_display();
}

static void click_down(ClickRecognizerRef r, void *ctx) {
    (void)r; (void)ctx;
    s_offset_minutes -= 15;
    update_display();
}

static void click_select(ClickRecognizerRef r, void *ctx) {
    (void)r; (void)ctx;
    EditStartResult res = apply_fast_start_offset(s_offset_minutes);
    if (res == EDIT_START_OK) {
        APP_LOG(APP_LOG_LEVEL_INFO, "edit-start-ok");
        window_stack_pop(true);
        return;
    }
    if (res == EDIT_START_FUTURE) {
        APP_LOG(APP_LOG_LEVEL_INFO, "edit-start-future");
        text_layer_set_text(s_error_label, "Too late");
    } else {
        APP_LOG(APP_LOG_LEVEL_INFO, "edit-start-old");
        text_layer_set_text(s_error_label, "Too old");
    }
    vibes_short_pulse();
}

static void click_config(void *ctx) {
    (void)ctx;
    window_single_click_subscribe(BUTTON_ID_UP,     click_up);
    window_single_click_subscribe(BUTTON_ID_DOWN,   click_down);
    window_single_click_subscribe(BUTTON_ID_SELECT, click_select);
}

static void window_load(Window *win) {
    Layer *root   = window_get_root_layer(win);
    GRect  bounds = layer_get_bounds(root);
    window_set_background_color(win, GColorBlack);

    // Show current recorded start time as context.
    static char header_buf[32];
    int32_t started = storage_get_fast_started_at();
    struct tm *t = localtime((const time_t *)&started);
    snprintf(header_buf, sizeof(header_buf), "Started %02d:%02d", t->tm_hour, t->tm_min);

    s_header = text_layer_create(GRect(4, 8, bounds.size.w - 8, 22));
    text_layer_set_background_color(s_header, GColorClear);
    text_layer_set_text_color(s_header, GColorLightGray);
    text_layer_set_font(s_header, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text_alignment(s_header, GTextAlignmentCenter);
    text_layer_set_text(s_header, header_buf);
    layer_add_child(root, text_layer_get_layer(s_header));

    s_offset_label = text_layer_create(GRect(4, bounds.size.h / 2 - 24, bounds.size.w - 8, 48));
    text_layer_set_background_color(s_offset_label, GColorClear);
    text_layer_set_text_color(s_offset_label, GColorWhite);
    text_layer_set_font(s_offset_label, fonts_get_system_font(FONT_KEY_LECO_36_BOLD_NUMBERS));
    text_layer_set_text_alignment(s_offset_label, GTextAlignmentCenter);
    layer_add_child(root, text_layer_get_layer(s_offset_label));

    s_error_label = text_layer_create(GRect(4, bounds.size.h - 36, bounds.size.w - 8, 18));
    text_layer_set_background_color(s_error_label, GColorClear);
#ifdef PBL_COLOR
    text_layer_set_text_color(s_error_label, GColorRed);
#else
    text_layer_set_text_color(s_error_label, GColorWhite);
#endif
    text_layer_set_font(s_error_label, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
    text_layer_set_text_alignment(s_error_label, GTextAlignmentCenter);
    layer_add_child(root, text_layer_get_layer(s_error_label));

    s_offset_minutes = 0;
    update_display();
    window_set_click_config_provider(win, click_config);
}

static void window_unload(Window *win) {
    (void)win;
    text_layer_destroy(s_header);
    text_layer_destroy(s_offset_label);
    text_layer_destroy(s_error_label);
    window_destroy(s_window);
    s_window = NULL;
}

void ui_edit_start_push(void) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers){
        .load   = window_load,
        .unload = window_unload,
    });
    window_stack_push(s_window, true);
}
