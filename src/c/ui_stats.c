// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0

#include "ui_stats.h"
#include "fasting.h"
#include "storage.h"

#include <pebble.h>
#include <stdio.h>

// ── Stats window ──────────────────────────────────────────────────────────────

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

    APP_LOG(APP_LOG_LEVEL_INFO, "stats-total-fasts:%lu", (unsigned long)st.total_fasts);
    snprintf(s_body_buf, sizeof(s_body_buf),
             "Fasts: %lu\nAvg:   %s\nLongest:\n  %s\nOvertime:\n  %s",
             (unsigned long)st.total_fasts,
             avg_buf, longest_buf, overtime_buf);
}

// ── Confirm-reset window ──────────────────────────────────────────────────────

#define CONFIRM_ROW_CANCEL 0
#define CONFIRM_ROW_RESET  1
#define CONFIRM_NUM_ROWS   2

static Window    *s_confirm_window;
static MenuLayer *s_confirm_menu;

static uint16_t confirm_get_num_rows(MenuLayer *ml, uint16_t section, void *ctx) {
    (void)ml; (void)section; (void)ctx;
    return CONFIRM_NUM_ROWS;
}

static void confirm_draw_row(GContext *gctx, const Layer *cell_layer, MenuIndex *idx, void *ctx) {
    (void)ctx;
    switch (idx->row) {
        case CONFIRM_ROW_CANCEL:
            menu_cell_basic_draw(gctx, cell_layer, "Cancel", "Go back", NULL);
            break;
        case CONFIRM_ROW_RESET:
            menu_cell_basic_draw(gctx, cell_layer, "Reset", "Clear all stats", NULL);
            break;
    }
}

static void confirm_on_select(MenuLayer *ml, MenuIndex *idx, void *ctx) {
    (void)ml; (void)ctx;
    if (idx->row == CONFIRM_ROW_RESET) {
        storage_reset_stats();
        APP_LOG(APP_LOG_LEVEL_INFO, "stats-reset");
        window_stack_pop(false);
        window_stack_pop(true);
    } else {
        window_stack_pop(true);
    }
}

static void confirm_window_load(Window *win) {
    Layer *root   = window_get_root_layer(win);
    GRect  bounds = layer_get_bounds(root);
    s_confirm_menu = menu_layer_create(bounds);
    menu_layer_set_callbacks(s_confirm_menu, NULL, (MenuLayerCallbacks){
        .get_num_rows = confirm_get_num_rows,
        .draw_row     = confirm_draw_row,
        .select_click = confirm_on_select,
    });
    menu_layer_set_click_config_onto_window(s_confirm_menu, win);
    layer_add_child(root, menu_layer_get_layer(s_confirm_menu));
}

static void confirm_window_unload(Window *win) {
    (void)win;
    menu_layer_destroy(s_confirm_menu);
    s_confirm_menu = NULL;
    window_destroy(s_confirm_window);
    s_confirm_window = NULL;
}

static void push_confirm_window(void) {
    s_confirm_window = window_create();
    window_set_window_handlers(s_confirm_window, (WindowHandlers){
        .load   = confirm_window_load,
        .unload = confirm_window_unload,
    });
    window_stack_push(s_confirm_window, true);
}

// ── Stats window handlers ─────────────────────────────────────────────────────

static void click_select(ClickRecognizerRef r, void *ctx) {
    (void)r; (void)ctx;
    push_confirm_window();
}

static void click_config(void *ctx) {
    (void)ctx;
    window_single_click_subscribe(BUTTON_ID_SELECT, click_select);
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

    window_set_click_config_provider(win, click_config);
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
