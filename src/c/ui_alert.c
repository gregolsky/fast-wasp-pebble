// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0

#include "ui_alert.h"

#include <pebble.h>
#include <stdio.h>

typedef struct {
    const char *title;
    const char *body;
    const char *hint; // NULL for milestone events
    uint32_t    icon; // resource id
} AlertCopy;

static const AlertCopy COPY[NOTIFY_EVENT_COUNT] = {
    [NOTIFY_FAST_COMPLETE]     = { "Sting!",    "You did it. Fast complete.",   "SELECT \xe2\x86\x92 eat",      RESOURCE_ID_IMAGE_ALERT_TROPHY    },
    [NOTIFY_EAT_WINDOW_CLOSED] = { "Hive shut", "Eat window closed. Reset.",    "SELECT \xe2\x86\x92 fast",     RESOURCE_ID_IMAGE_ALERT_LOCK      },
    [NOTIFY_OMAD_TARGET]       = { "OMAD",       "23h. Meal time.",              "SELECT \xe2\x86\x92 log meal", RESOURCE_ID_IMAGE_ALERT_PLATE     },
    [NOTIFY_FAST_HALFWAY]      = { "Halfway",    "You've got this. Keep flying.", NULL,                          RESOURCE_ID_IMAGE_ALERT_MUSCLE    },
    [NOTIFY_FAST_1H_LEFT]      = { "60 min",     "One hour. Don't blink.",       NULL,                          RESOURCE_ID_IMAGE_ALERT_HOURGLASS },
    [NOTIFY_KETOSIS_START]     = { "Ketosis",    "Fat is fuel. Stay sharp.",     NULL,                          RESOURCE_ID_IMAGE_ALERT_FLAME     },
    [NOTIFY_AUTOPHAGY_START]   = { "Autophagy",  "Deep work. Cells clearing.",   NULL,                          RESOURCE_ID_IMAGE_ALERT_SPARKLES  },
};

// ── Window state ──────────────────────────────────────────────────────────────

static Window    *s_window;
static TextLayer *s_title_layer;
static TextLayer *s_body_layer;
static TextLayer *s_hint_layer;
static BitmapLayer *s_icon_layer;
static GBitmap     *s_icon;
static const AlertCopy *s_copy;
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

    // Round screens: centre text and pull it in from the bezel.
    int inset = PBL_IF_ROUND_ELSE(30, 6);
    GTextAlignment align = PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft);

#ifdef PBL_COLOR
    int header_h = PBL_IF_ROUND_ELSE(64, 44);
    s_header_layer = layer_create(GRect(0, 0, bounds.size.w, header_h));
    layer_set_update_proc(s_header_layer, header_draw);
    layer_add_child(root, s_header_layer);

    s_title_layer = text_layer_create(GRect(inset, PBL_IF_ROUND_ELSE(26, 8), bounds.size.w - 2 * inset, header_h - PBL_IF_ROUND_ELSE(30, 12)));
    text_layer_set_background_color(s_title_layer, GColorClear);
    text_layer_set_text_color(s_title_layer, GColorBlack);
    text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_text_alignment(s_title_layer, align);
    text_layer_set_text(s_title_layer, s_copy->title);
    layer_add_child(root, text_layer_get_layer(s_title_layer));

    int body_y = header_h + 6;
#else
    s_title_layer = text_layer_create(GRect(inset, 10, bounds.size.w - 2 * inset, 30));
    text_layer_set_background_color(s_title_layer, GColorClear);
    text_layer_set_text_color(s_title_layer, GColorWhite);
    text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_text_alignment(s_title_layer, align);
    text_layer_set_text(s_title_layer, s_copy->title);
    layer_add_child(root, text_layer_get_layer(s_title_layer));

    int body_y = 44;
#endif
    // Round: lift the hint out of the narrow bottom of the circle.
    int hint_y = bounds.size.h - PBL_IF_ROUND_ELSE(46, 20);
    // Icon size comes from the per-platform resource (see tools/gen_alert_icons.py).
    GSize icon = gbitmap_get_bounds(s_icon).size;
    int icon_max_y = hint_y - icon.h - 4;
    s_body_layer = text_layer_create(GRect(inset, body_y, bounds.size.w - 2 * inset, icon_max_y - body_y));
    text_layer_set_text(s_body_layer, s_copy->body);
    text_layer_set_background_color(s_body_layer, GColorClear);
    text_layer_set_text_color(s_body_layer, GColorLightGray);
    text_layer_set_font(s_body_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(s_body_layer, align);
    text_layer_set_overflow_mode(s_body_layer, GTextOverflowModeWordWrap);
    layer_add_child(root, text_layer_get_layer(s_body_layer));

    // Centre the icon in the gap between the wrapped body text and the hint.
    int text_bottom = body_y + text_layer_get_content_size(s_body_layer).h;
    int icon_y = text_bottom + (icon_max_y - text_bottom) / 2;
    s_icon_layer = bitmap_layer_create(GRect((bounds.size.w - icon.w) / 2, icon_y, icon.w, icon.h));
    bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
    bitmap_layer_set_bitmap(s_icon_layer, s_icon);
    layer_add_child(root, bitmap_layer_get_layer(s_icon_layer));

    s_hint_layer = text_layer_create(GRect(inset, hint_y, bounds.size.w - 2 * inset, 18));
    text_layer_set_background_color(s_hint_layer, GColorClear);
    // DarkGray is black on 1-bit screens, which hid the hint there.
    text_layer_set_text_color(s_hint_layer, PBL_IF_COLOR_ELSE(GColorLightGray, GColorWhite));
    text_layer_set_text(s_hint_layer, s_copy->hint ? s_copy->hint : "");
    text_layer_set_font(s_hint_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_text_alignment(s_hint_layer, align);
    layer_add_child(root, text_layer_get_layer(s_hint_layer));

    window_set_click_config_provider(win, click_config);
}

static void window_unload(Window *win) {
    (void)win;
    text_layer_destroy(s_title_layer);
    text_layer_destroy(s_body_layer);
    text_layer_destroy(s_hint_layer);
    bitmap_layer_destroy(s_icon_layer);
    gbitmap_destroy(s_icon);
    s_icon = NULL;
#ifdef PBL_COLOR
    layer_destroy(s_header_layer);
#endif
    window_destroy(s_window);
    s_window = NULL;
}

// ── Public API ────────────────────────────────────────────────────────────────

void ui_alert_push(NotifyEvent event) {
    if ((int)event >= NOTIFY_EVENT_COUNT) return;
    s_copy = &COPY[(int)event];
    APP_LOG(APP_LOG_LEVEL_INFO, "alert:%d", (int)event);

    s_icon = gbitmap_create_with_resource(s_copy->icon); // window_load needs its size
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers){
        .load   = window_load,
        .unload = window_unload,
    });
    window_stack_push(s_window, true);
}
