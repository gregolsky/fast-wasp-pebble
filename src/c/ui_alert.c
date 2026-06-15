// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0

#include "ui_alert.h"

#include <pebble.h>
#include <stdio.h>

typedef struct {
    const char *title;
    const char *body;
    const char *hint; // NULL for milestone events
} AlertCopy;

static const AlertCopy COPY[NOTIFY_EVENT_COUNT] = {
    [NOTIFY_FAST_COMPLETE]     = { "Sting!",    "You did it. Fast complete.",   "SELECT \xe2\x86\x92 eat"      },
    [NOTIFY_EAT_WINDOW_CLOSED] = { "Hive shut", "Eat window closed. Reset.",    "SELECT \xe2\x86\x92 fast"     },
    [NOTIFY_OMAD_TARGET]       = { "OMAD",       "23h. Meal time.",              "SELECT \xe2\x86\x92 log meal" },
    [NOTIFY_FAST_HALFWAY]      = { "Halfway",    "You've got this. Keep flying.", NULL                          },
    [NOTIFY_FAST_1H_LEFT]      = { "60 min",     "One hour. Don't blink.",       NULL                          },
    [NOTIFY_KETOSIS_START]     = { "Ketosis",    "Fat is fuel. Stay sharp.",     NULL                          },
    [NOTIFY_AUTOPHAGY_START]   = { "Autophagy",  "Deep work. Cells clearing.",   NULL                          },
};

// ── Window state ──────────────────────────────────────────────────────────────

static Window    *s_window;
static TextLayer *s_title_layer;
static TextLayer *s_body_layer;
static TextLayer *s_hint_layer;
#ifdef PBL_COLOR
static Layer     *s_header_layer;
#endif

#ifdef PBL_COLOR
static void header_draw(Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorYellow);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}
#endif

static void click_dismiss(ClickRecognizerRef r, void *ctx) {
    (void)r; (void)ctx;
    window_stack_remove(s_window, true);
}

static void click_config(void *ctx) {
    (void)ctx;
    window_single_click_subscribe(BUTTON_ID_SELECT, click_dismiss);
    window_single_click_subscribe(BUTTON_ID_BACK,   click_dismiss);
}

static void window_load(Window *win) {
    Layer *root   = window_get_root_layer(win);
    GRect  bounds = layer_get_bounds(root);

    window_set_background_color(win, GColorBlack);

#ifdef PBL_COLOR
    int header_h = 44;
    s_header_layer = layer_create(GRect(0, 0, bounds.size.w, header_h));
    layer_set_update_proc(s_header_layer, header_draw);
    layer_add_child(root, s_header_layer);

    s_title_layer = text_layer_create(GRect(6, 8, bounds.size.w - 12, header_h - 12));
    text_layer_set_background_color(s_title_layer, GColorClear);
    text_layer_set_text_color(s_title_layer, GColorBlack);
    text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_text_alignment(s_title_layer, GTextAlignmentLeft);
    layer_add_child(root, text_layer_get_layer(s_title_layer));

    s_body_layer = text_layer_create(GRect(6, header_h + 6, bounds.size.w - 12, bounds.size.h - header_h - 30));
#else
    s_title_layer = text_layer_create(GRect(6, 10, bounds.size.w - 12, 30));
    text_layer_set_background_color(s_title_layer, GColorClear);
    text_layer_set_text_color(s_title_layer, GColorWhite);
    text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_text_alignment(s_title_layer, GTextAlignmentLeft);
    layer_add_child(root, text_layer_get_layer(s_title_layer));

    s_body_layer = text_layer_create(GRect(6, 44, bounds.size.w - 12, bounds.size.h - 66));
#endif
    text_layer_set_background_color(s_body_layer, GColorClear);
    text_layer_set_text_color(s_body_layer, GColorLightGray);
    text_layer_set_font(s_body_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(s_body_layer, GTextAlignmentLeft);
    text_layer_set_overflow_mode(s_body_layer, GTextOverflowModeWordWrap);
    layer_add_child(root, text_layer_get_layer(s_body_layer));

    s_hint_layer = text_layer_create(GRect(6, bounds.size.h - 20, bounds.size.w - 12, 18));
    text_layer_set_background_color(s_hint_layer, GColorClear);
    text_layer_set_text_color(s_hint_layer, GColorDarkGray);
    text_layer_set_font(s_hint_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_text_alignment(s_hint_layer, GTextAlignmentLeft);
    layer_add_child(root, text_layer_get_layer(s_hint_layer));

    window_set_click_config_provider(win, click_config);
}

static void window_unload(Window *win) {
    (void)win;
    text_layer_destroy(s_title_layer);
    text_layer_destroy(s_body_layer);
    text_layer_destroy(s_hint_layer);
#ifdef PBL_COLOR
    layer_destroy(s_header_layer);
#endif
    window_destroy(s_window);
    s_window = NULL;
}

// ── Public API ────────────────────────────────────────────────────────────────

void ui_alert_push(NotifyEvent event) {
    if ((int)event >= NOTIFY_EVENT_COUNT) return;
    const AlertCopy *c = &COPY[(int)event];

    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers){
        .load   = window_load,
        .unload = window_unload,
    });
    window_stack_push(s_window, true);

    text_layer_set_text(s_title_layer, c->title);
    text_layer_set_text(s_body_layer,  c->body);
    text_layer_set_text(s_hint_layer,  c->hint ? c->hint : "");
}
