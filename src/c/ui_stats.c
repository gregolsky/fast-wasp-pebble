// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0

#include "ui_stats.h"
#include "fasting.h"

#include <pebble.h>
#include <stdio.h>
#include <string.h>

static Window      *s_window;
static ScrollLayer *s_scroll;
static TextLayer   *s_text;

static char s_buf[256];

static void build_text(void) {
    FastStats st = compute_fast_stats();
    APP_LOG(APP_LOG_LEVEL_INFO, "stats-total-fasts:%lu", (unsigned long)st.total_fasts);

    char avg_buf[16], longest_buf[16], overtime_buf[16];
    fast_format_hms((int32_t)st.avg_seconds,             avg_buf);
    fast_format_hms((int32_t)st.longest_seconds,          longest_buf);
    fast_format_hms((int32_t)st.total_overtime_seconds,   overtime_buf);

    snprintf(s_buf, sizeof(s_buf),
             "Total fasts\n%lu\n\nAverage\n%s\n\nLongest\n%s\n\nTotal overtime\n%s",
             (unsigned long)st.total_fasts,
             avg_buf, longest_buf, overtime_buf);
}

static void window_load(Window *win) {
    Layer *root   = window_get_root_layer(win);
    GRect  bounds = layer_get_bounds(root);
    window_set_background_color(win, GColorBlack);

    build_text();

    // Measure text height so the scroll layer knows how tall to make its content.
    GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
    GSize content_size = GSize(bounds.size.w - 8,
                               graphics_text_layout_get_content_size(
                                   s_buf, font,
                                   GRect(0, 0, bounds.size.w - 8, 2000),
                                   GTextOverflowModeWordWrap,
                                   GTextAlignmentLeft).h + 16);

    s_scroll = scroll_layer_create(bounds);
    scroll_layer_set_content_size(s_scroll, content_size);
    scroll_layer_set_click_config_onto_window(s_scroll, win);
    layer_add_child(root, scroll_layer_get_layer(s_scroll));

    s_text = text_layer_create(GRect(4, 4, content_size.w, content_size.h));
    text_layer_set_background_color(s_text, GColorClear);
    text_layer_set_text_color(s_text, GColorWhite);
    text_layer_set_font(s_text, font);
    text_layer_set_overflow_mode(s_text, GTextOverflowModeWordWrap);
    text_layer_set_text(s_text, s_buf);
    scroll_layer_add_child(s_scroll, text_layer_get_layer(s_text));
}

static void window_unload(Window *win) {
    (void)win;
    text_layer_destroy(s_text);
    scroll_layer_destroy(s_scroll);
    window_destroy(s_window);
    s_window = NULL;
}

void ui_stats_push(void) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers){
        .load   = window_load,
        .unload = window_unload,
    });
    window_stack_push(s_window, true);
}
