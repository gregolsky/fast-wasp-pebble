// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0

#include "ui_settings.h"
#include "version.h"
#include "storage.h"
#include "notify.h"

#include <pebble.h>
#include <stdio.h>

#define ROW_VIBRATION 0
#define ROW_RESET     1
#define ROW_ABOUT     2
#define NUM_ROWS      3

static Window    *s_window;
static MenuLayer *s_menu;
static bool       s_confirm_reset;

static uint16_t get_num_rows(MenuLayer *ml, uint16_t section, void *ctx) {
    (void)ml; (void)section; (void)ctx;
    return NUM_ROWS;
}

static void draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *idx, void *context) {
    (void)context;
    switch (idx->row) {
        case ROW_VIBRATION: {
            char sub[16];
            snprintf(sub, sizeof(sub), "%s", storage_get_vibration_on() ? "On" : "Off");
            menu_cell_basic_draw(ctx, cell_layer, "Vibration", sub, NULL);
            break;
        }
        case ROW_RESET:
            menu_cell_basic_draw(ctx, cell_layer,
                                 s_confirm_reset ? "Confirm Reset?" : "Reset Data",
                                 s_confirm_reset ? "Press again" : "Wipe all data",
                                 NULL);
            break;
        case ROW_ABOUT:
            menu_cell_basic_draw(ctx, cell_layer, "About", "Fast Pebble v" APP_VERSION, NULL);
            break;
    }
}

static void on_select(MenuLayer *ml, MenuIndex *idx, void *ctx) {
    (void)ml; (void)ctx;
    switch (idx->row) {
        case ROW_VIBRATION:
            storage_set_vibration_on(!storage_get_vibration_on());
            menu_layer_reload_data(s_menu);
            break;
        case ROW_RESET:
            if (s_confirm_reset) {
                notify_cancel();
                storage_reset_all();
                APP_LOG(APP_LOG_LEVEL_INFO, "storage-reset");
                window_stack_pop_all(true);
            } else {
                s_confirm_reset = true;
                menu_layer_reload_data(s_menu);
            }
            break;
        case ROW_ABOUT:
            break;
    }
}

static void window_load(Window *win) {
    Layer *root   = window_get_root_layer(win);
    GRect  bounds = layer_get_bounds(root);
    s_confirm_reset = false;
    s_menu = menu_layer_create(bounds);
    menu_layer_set_callbacks(s_menu, NULL, (MenuLayerCallbacks){
        .get_num_rows = get_num_rows,
        .draw_row     = draw_row,
        .select_click = on_select,
    });
    menu_layer_set_click_config_onto_window(s_menu, win);
    layer_add_child(root, menu_layer_get_layer(s_menu));
}

static void window_unload(Window *win) {
    (void)win;
    menu_layer_destroy(s_menu);
    s_menu = NULL;
    window_destroy(s_window);
    s_window = NULL;
}

void ui_settings_push(void) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers){
        .load   = window_load,
        .unload = window_unload,
    });
    window_stack_push(s_window, true);
}
