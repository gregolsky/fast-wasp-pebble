// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0

#include "ui_stats.h"
#include "fasting.h"

#include <pebble.h>
#include <stdio.h>

static Window    *s_window;
static TextLayer *s_title;
static TextLayer *s_body;

static char s_body_buf[128];

static void build_body(void) {
    FastStats st = compute_fast_stats();

    char avg_buf[16], longest_buf[16], overtime_buf[16];
    fast_format_hms((int32_t)st.avg_seconds,           avg_buf);
    fast_format_hms((int32_t)st.longest_seconds,        longest_buf);
    fast_format_hms((int32_t)st.total_overtime_seconds, overtime_buf);

    snprintf(s_body_buf, sizeof(s_body_buf),
             "Fasts: %lu\nAvg:   %s\nLongest:\n  %s\nOvertime:\n  %s",
             (unsigned long)st.total_fasts,
             avg_buf, longest_buf, overtime_buf);
}

static void window_load(Window *win) {
    Layer *root   = window_get_root_layer(win);
    GRect  bounds = layer_get_bounds(root);
    window_set_background_color(win, GColorBlack);

    s_title = text_layer_create(GRect(0, 4, bounds.size.w, 22));
    text_layer_set_background_color(s_title, GColorClear);
    text_layer_set_text_color(s_title, GColorWhite);
    text_layer_set_font(s_title, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(s_title, GTextAlignmentCenter);
    text_layer_set_text(s_title, "Stats");
    layer_add_child(root, text_layer_get_layer(s_title));

    build_body();

    s_body = text_layer_create(GRect(8, 30, bounds.size.w - 16, bounds.size.h - 34));
    text_layer_set_background_color(s_body, GColorClear);
    text_layer_set_text_color(s_body, GColorWhite);
    text_layer_set_font(s_body, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_overflow_mode(s_body, GTextOverflowModeWordWrap);
    text_layer_set_text(s_body, s_body_buf);
    layer_add_child(root, text_layer_get_layer(s_body));
}

static void window_unload(Window *win) {
    (void)win;
    text_layer_destroy(s_title);
    text_layer_destroy(s_body);
    s_window = NULL;
}

void ui_stats_push(void) {
    if (s_window) {
        window_destroy(s_window);
        s_window = NULL;
    }
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers){
        .load   = window_load,
        .unload = window_unload,
    });
    window_stack_push(s_window, true);
}
