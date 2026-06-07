// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0

#include "ui_picker.h"
#include "fasting.h"
#include "ui_settings.h"

#include <stdio.h>
#include <string.h>

static Window    *s_window;
static MenuLayer *s_menu;
static void     (*s_on_select)(uint8_t);

static uint16_t get_num_rows(MenuLayer *ml, uint16_t section, void *ctx) {
    (void)ml; (void)section; (void)ctx;
    return NUM_PROGRAMS + 1;
}

static void draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *idx, void *context) {
    (void)context;
    if (idx->row == NUM_PROGRAMS) {
        menu_cell_basic_draw(ctx, cell_layer, "Settings", NULL, NULL);
        return;
    }
    const Program *p = program_by_index((uint8_t)idx->row);
    char buf[32];
    if (p->sub[0]) {
        snprintf(buf, sizeof(buf), "%s", p->sub);
        menu_cell_basic_draw(ctx, cell_layer, p->label, buf, NULL);
    } else {
        menu_cell_basic_draw(ctx, cell_layer, p->label, NULL, NULL);
    }
}

static void on_select_click(MenuLayer *ml, MenuIndex *idx, void *ctx) {
    (void)ml; (void)ctx;
    if (idx->row == NUM_PROGRAMS) {
        ui_settings_push();
        return;
    }
    window_stack_pop(true);
    if (s_on_select) s_on_select((uint8_t)idx->row);
}

static void click_up(ClickRecognizerRef r, void *ctx) {
    (void)r; (void)ctx;
    menu_layer_set_selected_next(s_menu, true, MenuRowAlignCenter, true);
}

static void click_down(ClickRecognizerRef r, void *ctx) {
    (void)r; (void)ctx;
    menu_layer_set_selected_next(s_menu, false, MenuRowAlignCenter, true);
}

static void click_select(ClickRecognizerRef r, void *ctx) {
    (void)r; (void)ctx;
    MenuIndex idx = menu_layer_get_selected_index(s_menu);
    on_select_click(s_menu, &idx, NULL);
}

static void click_config(void *ctx) {
    (void)ctx;
    window_single_click_subscribe(BUTTON_ID_UP,     click_up);
    window_single_click_subscribe(BUTTON_ID_DOWN,   click_down);
    window_single_click_subscribe(BUTTON_ID_SELECT, click_select);
}

static void window_load(Window *win) {
    Layer *root = window_get_root_layer(win);
    GRect bounds = layer_get_bounds(root);
    s_menu = menu_layer_create(bounds);
    menu_layer_set_callbacks(s_menu, NULL, (MenuLayerCallbacks){
        .get_num_rows      = get_num_rows,
        .draw_row          = draw_row,
        .select_click      = on_select_click,
    });
    window_set_click_config_provider_with_context(win, click_config, NULL);
    layer_add_child(root, menu_layer_get_layer(s_menu));
}

static void window_unload(Window *win) {
    (void)win;
    menu_layer_destroy(s_menu);
    s_menu = NULL;
    window_destroy(s_window);
    s_window = NULL;
}

void ui_picker_push(void (*on_select)(uint8_t program_idx)) {
    s_on_select = on_select;
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers){
        .load   = window_load,
        .unload = window_unload,
    });
    window_stack_push(s_window, true);
}
