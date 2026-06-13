// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0

#include "ui_fast.h"
#include "ui_picker.h"
#include "ui_stats.h"
#include "ui_settings.h"
#include "fasting.h"
#include "storage.h"
#include "notify.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

// ── Shared state ──────────────────────────────────────────────────────────────

static Window      *s_window;
static Layer       *s_ring_layer;
static TextLayer   *s_mode_label;
static TextLayer   *s_program_label;
static TextLayer   *s_time_label;
static TextLayer   *s_overtime_label;
static TextLayer   *s_ends_at_label;
static TextLayer   *s_hint_label;
static AppTimer    *s_tick_timer;

static FastUIState  s_state;
static bool         s_fast_complete_fired;
static bool         s_eat_complete_fired;

static char s_mode_buf[16];
static char s_prog_buf[24];
static char s_time_buf[16];
static char s_ends_buf[16];
static char s_hint_buf[24];

static int  s_banner_ticks;
static char s_banner_buf[40];

// ── Ring drawing ──────────────────────────────────────────────────────────────

static int32_t s_ring_pct;    // 0..100; updated each tick

static void ring_layer_draw(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    GPoint centre = GPoint(bounds.size.w / 2, bounds.size.h / 2);
    uint16_t radius = (bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h) / 2 - 4;

    // Background ring
    graphics_context_set_stroke_color(ctx, GColorDarkGray);
    graphics_context_set_stroke_width(ctx, 6);
    graphics_draw_circle(ctx, centre, radius);

    // Progress arc (clockwise from 12 o'clock = -90 degrees in Pebble coords)
    if (s_ring_pct > 0) {
#ifdef PBL_COLOR
        graphics_context_set_stroke_color(ctx, GColorYellow);
#else
        graphics_context_set_stroke_color(ctx, GColorWhite);
#endif
        int32_t angle = (int32_t)(TRIG_MAX_ANGLE * s_ring_pct / 100);
        graphics_draw_arc(ctx, grect_inset(bounds, GEdgeInsets(4)),
                          GOvalScaleModeFitCircle,
                          DEG_TO_TRIGANGLE(-90),
                          DEG_TO_TRIGANGLE(-90) + angle);
    }
}

// ── Tick update ───────────────────────────────────────────────────────────────

static void schedule_tick(void);

static void tick_cb(void *ctx) {
    (void)ctx;
    if (s_banner_ticks > 0) s_banner_ticks--;
    ui_fast_refresh();
    schedule_tick();
}

static void schedule_tick(void) {
    s_tick_timer = app_timer_register(1000, tick_cb, NULL);
}

static void cancel_tick(void) {
    if (s_tick_timer) {
        app_timer_cancel(s_tick_timer);
        s_tick_timer = NULL;
    }
}

// ── State detection ───────────────────────────────────────────────────────────

static FastUIState detect_state(void) {
    uint8_t prog = storage_get_program_id();
    if (prog == 0xFF) return FAST_STATE_READY;  // no program set

    const Program *p = program_by_index(prog);
    if (p->is_omad) {
        return storage_get_omad_last_meal_at() != 0 ? FAST_STATE_OMAD : FAST_STATE_READY;
    }
    if (storage_get_fast_started_at() != 0) return FAST_STATE_ACTIVE;
    if (storage_get_eat_started_at()  != 0) return FAST_STATE_EATING;
    return FAST_STATE_READY;
}

// ── Refresh (called every tick and on state change) ───────────────────────────

void ui_fast_refresh(void) {
    if (!s_window) return;

    s_state = detect_state();
    uint8_t prog_idx = storage_get_program_id();

    bool show_overtime = false;

    switch (s_state) {
        case FAST_STATE_READY: {
            if (prog_idx == 0xFF) {
                s_mode_buf[0] = '\0';
                snprintf(s_prog_buf, sizeof(s_prog_buf), "Select Program");
                snprintf(s_hint_buf, sizeof(s_hint_buf), "Hold SEL to pick");
            } else {
                const Program *p = program_by_index(prog_idx);
                snprintf(s_mode_buf, sizeof(s_mode_buf), "Ready");
                snprintf(s_prog_buf, sizeof(s_prog_buf), "%s", p->label);
                snprintf(s_hint_buf, sizeof(s_hint_buf), "SEL: START");
            }
            snprintf(s_time_buf, sizeof(s_time_buf), "Ready");
            s_ends_buf[0] = '\0';
            s_ring_pct = 0;
            APP_LOG(APP_LOG_LEVEL_INFO, "state-ready");
            break;
        }

        case FAST_STATE_ACTIVE: {
            int32_t started    = storage_get_fast_started_at();
            uint8_t tgt_hours  = storage_get_fast_target_hours();
            int32_t rem        = fast_remaining(started, tgt_hours);
            int32_t el         = fast_elapsed(started);
            int32_t target_sec = (int32_t)tgt_hours * 3600;

            show_overtime = rem < 0;
            if (rem > -1800 && rem < 1800)
                fast_format_hms(rem, s_time_buf);
            else
                fast_format_hm(rem, s_time_buf);
            s_ring_pct = target_sec > 0 ? (int32_t)(el * 100 / target_sec) : 0;
            if (s_ring_pct > 100) s_ring_pct = 100;

            // Compute wall-clock fast-end time.
            time_t end_ts = (time_t)(started + target_sec);
            struct tm *lt = localtime(&end_ts);
            if (lt) {
                if (clock_is_24h_style()) {
                    strftime(s_ends_buf, sizeof(s_ends_buf), "ends %H:%M", lt);
                } else {
                    strftime(s_ends_buf, sizeof(s_ends_buf), "ends %l:%M%p", lt);
                }
            } else {
                s_ends_buf[0] = '\0';
            }

            const Program *p = program_by_index(prog_idx);
            snprintf(s_mode_buf, sizeof(s_mode_buf), "Fasting");
            snprintf(s_prog_buf, sizeof(s_prog_buf), "%s", p->label);
            snprintf(s_hint_buf, sizeof(s_hint_buf), "SEL: STOP");
            APP_LOG(APP_LOG_LEVEL_INFO, "fast-active");

            // Fire wakeup notification exactly once per session when target is hit.
            if (show_overtime && !s_fast_complete_fired) {
                s_fast_complete_fired = true;
                APP_LOG(APP_LOG_LEVEL_INFO, "fast-complete");
            }
            break;
        }

        case FAST_STATE_EATING: {
            int32_t started    = storage_get_eat_started_at();
            uint8_t tgt_hours  = storage_get_eat_target_hours();
            int32_t rem        = fast_remaining(started, tgt_hours);
            int32_t el         = fast_elapsed(started);
            int32_t target_sec = (int32_t)tgt_hours * 3600;

            show_overtime = rem < 0;
            if (rem > -1800 && rem < 1800)
                fast_format_hms(rem, s_time_buf);
            else
                fast_format_hm(rem, s_time_buf);
            s_ring_pct = target_sec > 0 ? (int32_t)(el * 100 / target_sec) : 0;
            if (s_ring_pct > 100) s_ring_pct = 100;

            s_ends_buf[0] = '\0';
            const Program *p = program_by_index(prog_idx);
            snprintf(s_mode_buf, sizeof(s_mode_buf), "Eating");
            snprintf(s_prog_buf, sizeof(s_prog_buf), "%s", p->label);
            snprintf(s_hint_buf, sizeof(s_hint_buf), "SEL: FAST");
            APP_LOG(APP_LOG_LEVEL_INFO, "eat-active");

            if (show_overtime && !s_eat_complete_fired) {
                s_eat_complete_fired = true;
                APP_LOG(APP_LOG_LEVEL_INFO, "eat-complete");
            }
            break;
        }

        case FAST_STATE_OMAD: {
            int32_t last_meal  = storage_get_omad_last_meal_at();
            int32_t el         = fast_elapsed(last_meal);
            int32_t target_sec = 23 * 3600;

            show_overtime = el >= target_sec;
            fast_format_hm(el, s_time_buf);
            s_ring_pct = (int32_t)(el * 100 / target_sec);
            if (s_ring_pct > 100) s_ring_pct = 100;

            s_ends_buf[0] = '\0';
            snprintf(s_mode_buf, sizeof(s_mode_buf), "Fasting");
            snprintf(s_prog_buf, sizeof(s_prog_buf), "OMAD");
            snprintf(s_hint_buf, sizeof(s_hint_buf), "SEL: ATE");
            APP_LOG(APP_LOG_LEVEL_INFO, "omad-active");
            break;
        }
    }

    text_layer_set_text(s_mode_label,     s_mode_buf);
    text_layer_set_text(s_program_label,  s_prog_buf);
    text_layer_set_text(s_time_label,     s_time_buf);
    text_layer_set_text(s_ends_at_label,  s_ends_buf);
    text_layer_set_text(s_hint_label,     s_banner_ticks > 0 ? s_banner_buf : s_hint_buf);
    text_layer_set_text(s_overtime_label, show_overtime ? "OVERTIME" : "");
    layer_mark_dirty(s_ring_layer);
}

// ── Banner ────────────────────────────────────────────────────────────────────

void ui_fast_show_banner(NotifyEvent ev) {
    switch (ev) {
        case NOTIFY_FAST_COMPLETE:
            snprintf(s_banner_buf, sizeof(s_banner_buf), "Fast done! SEL=STOP");
            break;
        case NOTIFY_EAT_WINDOW_CLOSED:
            snprintf(s_banner_buf, sizeof(s_banner_buf), "Eat done! SEL=FAST");
            break;
        case NOTIFY_OMAD_TARGET:
            snprintf(s_banner_buf, sizeof(s_banner_buf), "OMAD target! SEL=ATE");
            break;
    }
    s_banner_ticks = 5;
    if (s_window) text_layer_set_text(s_hint_label, s_banner_buf);
}

// ── Button handlers ───────────────────────────────────────────────────────────

static void on_select_program(uint8_t idx) {
    storage_set_program_id(idx);
    storage_set_fast_started_at(0);
    storage_set_eat_started_at(0);
    storage_set_omad_last_meal_at(0);
    notify_cancel();
    s_fast_complete_fired = false;
    s_eat_complete_fired  = false;
    APP_LOG(APP_LOG_LEVEL_INFO, "program-selected");
    ui_fast_refresh();
}

static void click_select_short(ClickRecognizerRef r, void *ctx) {
    (void)r; (void)ctx;
    const Program *p = program_by_index(storage_get_program_id());

    switch (s_state) {
        case FAST_STATE_READY:
            if (storage_get_program_id() != 0xFF) {
                if (p->is_omad) {
                    log_meal();
                } else {
                    start_fast(storage_get_program_id());
                    s_fast_complete_fired = false;
                    int32_t st = storage_get_fast_started_at();
                    uint8_t th = storage_get_fast_target_hours();
                    notify_schedule(NOTIFY_FAST_COMPLETE, st + (int32_t)th * 3600);
                }
                vibes_short_pulse();
            }
            break;

        case FAST_STATE_ACTIVE:
            stop_fast();
            notify_cancel();
            if (storage_get_eat_started_at() != 0) {
                int32_t es = storage_get_eat_started_at();
                uint8_t eh = storage_get_eat_target_hours();
                notify_schedule(NOTIFY_EAT_WINDOW_CLOSED, es + (int32_t)eh * 3600);
                s_eat_complete_fired = false;
            }
            vibes_short_pulse();
            break;

        case FAST_STATE_EATING:
            eating_window_restart_fast();
            notify_cancel();
            {
                int32_t st = storage_get_fast_started_at();
                uint8_t th = storage_get_fast_target_hours();
                notify_schedule(NOTIFY_FAST_COMPLETE, st + (int32_t)th * 3600);
                s_fast_complete_fired = false;
            }
            vibes_short_pulse();
            break;

        case FAST_STATE_OMAD:
            log_meal();
            notify_cancel();
            notify_schedule(NOTIFY_OMAD_TARGET,
                            storage_get_omad_last_meal_at() + 23 * 3600);
            vibes_short_pulse();
            break;
    }
    (void)p;
    ui_fast_refresh();
}

static void click_select_long(ClickRecognizerRef r, void *ctx) {
    (void)r; (void)ctx;
    ui_picker_push(on_select_program);
}

static void click_up_short(ClickRecognizerRef r, void *ctx) {
    (void)r; (void)ctx;
    ui_stats_push();
}

static void click_down_short(ClickRecognizerRef r, void *ctx) {
    (void)r; (void)ctx;
    ui_settings_push();
}

static void click_config(void *ctx) {
    (void)ctx;
    window_single_click_subscribe(BUTTON_ID_SELECT, click_select_short);
    window_long_click_subscribe(BUTTON_ID_SELECT, 700, click_select_long, NULL);
    window_single_click_subscribe(BUTTON_ID_UP, click_up_short);
    window_single_click_subscribe(BUTTON_ID_DOWN, click_down_short);
}

// ── Window lifecycle ──────────────────────────────────────────────────────────

static void window_load(Window *win) {
    Layer *root   = window_get_root_layer(win);
    GRect  bounds = layer_get_bounds(root);

    window_set_background_color(win, GColorBlack);

    // Ring occupies the full window
    s_ring_layer = layer_create(bounds);
    layer_set_update_proc(s_ring_layer, ring_layer_draw);
    layer_add_child(root, s_ring_layer);

    // Mode label — top row (e.g. "Fasting", "Eating", "Ready")
    s_mode_label = text_layer_create(GRect(4, 20, bounds.size.w - 8, 16));
    text_layer_set_background_color(s_mode_label, GColorClear);
    text_layer_set_text_color(s_mode_label, GColorLightGray);
    text_layer_set_font(s_mode_label, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
    text_layer_set_text_alignment(s_mode_label, GTextAlignmentCenter);
    layer_add_child(root, text_layer_get_layer(s_mode_label));

    // Program label — second row (e.g. "16:8", "OMAD")
    s_program_label = text_layer_create(GRect(4, 34, bounds.size.w - 8, 22));
    text_layer_set_background_color(s_program_label, GColorClear);
    text_layer_set_text_color(s_program_label, GColorWhite);
    text_layer_set_font(s_program_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(s_program_label, GTextAlignmentCenter);
    layer_add_child(root, text_layer_get_layer(s_program_label));

    // Overtime badge
    s_overtime_label = text_layer_create(GRect(4, bounds.size.h / 2 - 32, bounds.size.w - 8, 20));
    text_layer_set_background_color(s_overtime_label, GColorClear);
#ifdef PBL_COLOR
    text_layer_set_text_color(s_overtime_label, GColorRed);
#else
    text_layer_set_text_color(s_overtime_label, GColorWhite);
#endif
    text_layer_set_font(s_overtime_label, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
    text_layer_set_text_alignment(s_overtime_label, GTextAlignmentCenter);
    layer_add_child(root, text_layer_get_layer(s_overtime_label));

    // Main time display — centre
    s_time_label = text_layer_create(GRect(4, bounds.size.h / 2 - 8, bounds.size.w - 8, 36));
    text_layer_set_background_color(s_time_label, GColorClear);
    text_layer_set_text_color(s_time_label, GColorWhite);
    text_layer_set_font(s_time_label, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
    text_layer_set_text_alignment(s_time_label, GTextAlignmentCenter);
    layer_add_child(root, text_layer_get_layer(s_time_label));

    // Fast-end wall-clock label — below countdown, ACTIVE state only
    s_ends_at_label = text_layer_create(GRect(4, bounds.size.h / 2 + 22, bounds.size.w - 8, 18));
    text_layer_set_background_color(s_ends_at_label, GColorClear);
    text_layer_set_text_color(s_ends_at_label, GColorLightGray);
    text_layer_set_font(s_ends_at_label, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_text_alignment(s_ends_at_label, GTextAlignmentCenter);
    layer_add_child(root, text_layer_get_layer(s_ends_at_label));

    // Hint row — bottom
    s_hint_label = text_layer_create(GRect(4, bounds.size.h - 20, bounds.size.w - 8, 18));
    text_layer_set_background_color(s_hint_label, GColorClear);
    text_layer_set_text_color(s_hint_label, GColorLightGray);
    text_layer_set_font(s_hint_label, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_text_alignment(s_hint_label, GTextAlignmentCenter);
    layer_add_child(root, text_layer_get_layer(s_hint_label));

    window_set_click_config_provider(win, click_config);

    s_fast_complete_fired = false;
    s_eat_complete_fired  = false;
    s_banner_ticks        = 0;
    ui_fast_refresh();
    schedule_tick();
}

static void window_unload(Window *win) {
    (void)win;
    cancel_tick();
    text_layer_destroy(s_mode_label);
    text_layer_destroy(s_program_label);
    text_layer_destroy(s_time_label);
    text_layer_destroy(s_ends_at_label);
    text_layer_destroy(s_overtime_label);
    text_layer_destroy(s_hint_label);
    layer_destroy(s_ring_layer);
    window_destroy(s_window);
    s_window = NULL;
}

void ui_fast_push(void) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers){
        .load   = window_load,
        .unload = window_unload,
    });
    window_stack_push(s_window, true);
}
