// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0

#include "fasting.h"
#include "storage.h"
#include "notify.h"
#include "platform/persist.h"
#include "platform/time.h"

#include <stdio.h>
#include <stdlib.h>

// ── Programs ──────────────────────────────────────────────────────────────────

const Program PROGRAMS[NUM_PROGRAMS] = {
    { "12:12", "Worker Wasp",   12, 12, false },
    { "14:10", "Scouting Wasp", 14, 10, false },
    { "16:8",  "Hunter Wasp",   16,  8, false },
    { "18:6",  "Soldier Wasp",  18,  6, false },
    { "20:4",  "Warrior Wasp",  20,  4, false },
    { "OMAD",  "Warrior Queen", OMAD_HOURS, 1, true  },
};

const Program *program_by_index(uint8_t idx) {
    if (idx >= NUM_PROGRAMS) return &PROGRAMS[0];
    return &PROGRAMS[idx];
}

// ── Timer arithmetic ──────────────────────────────────────────────────────────

int32_t fast_elapsed(int32_t started_at) {
    return time_now() - started_at;
}

int32_t fast_remaining(int32_t started_at, uint8_t target_hours) {
    return (int32_t)target_hours * 3600 - fast_elapsed(started_at);
}

bool fast_is_overtime(int32_t started_at, uint8_t target_hours) {
    return fast_remaining(started_at, target_hours) < 0;
}

void fast_format_hms(int32_t total_seconds, char *buf) {
    int32_t abs_s = total_seconds < 0 ? -total_seconds : total_seconds;
    int32_t h = abs_s / 3600;
    int32_t m = (abs_s % 3600) / 60;
    int32_t s = abs_s % 60;
    snprintf(buf, 16, "%02d:%02d:%02d", (int)h, (int)m, (int)s);
}

void fast_format_hm(int32_t total_seconds, char *buf) {
    int32_t abs_s = total_seconds < 0 ? -total_seconds : total_seconds;
    int32_t h = abs_s / 3600;
    int32_t m = (abs_s % 3600) / 60;
    snprintf(buf, 16, "%02d:%02d", (int)h, (int)m);
}

// ── Helpers ───────────────────────────────────────────────────────────────────

static void close_eating_window_into_history(void) {
    int32_t eat_started = storage_get_eat_started_at();
    uint8_t eat_hours   = storage_get_eat_target_hours();
    if (eat_started == 0) return;

    int32_t  now          = time_now();
    int32_t  actual_sec   = now - eat_started;
    int32_t  target_sec   = (int32_t)eat_hours * 3600;
    int32_t  overtime_sec = actual_sec > target_sec ? actual_sec - target_sec : 0;

    // Eating window closures are not tracked in fast history on Pebble.
    (void)overtime_sec;

    storage_set_eat_started_at(0);
    storage_set_eat_target_hours(0);
}

// ── Active fast ───────────────────────────────────────────────────────────────

void start_fast(uint8_t program_idx) {
    close_eating_window_into_history();

    const Program *p = program_by_index(program_idx);
    int32_t now = time_now();
    storage_set_fast_started_at(now);
    storage_set_fast_target_hours(p->fast_hours);
    storage_set_program_id(program_idx);
}

void stop_fast(void) {
    int32_t fast_started = storage_get_fast_started_at();
    if (fast_started == 0) return;

    int32_t  now          = time_now();
    int32_t  actual_sec   = now - fast_started;
    uint8_t  target_hours = storage_get_fast_target_hours();
    int32_t  target_sec   = (int32_t)target_hours * 3600;
    int32_t  overtime_sec = actual_sec > target_sec ? actual_sec - target_sec : 0;

    storage_record_fast(actual_sec, overtime_sec);

    storage_set_fast_started_at(0);
    storage_set_fast_target_hours(0);

    // Open eating window if the program has one.
    uint8_t  prog_idx = storage_get_program_id();
    const Program *p  = program_by_index(prog_idx);
    if (!p->is_omad && p->eat_hours > 0) {
        storage_set_eat_started_at(now);
        storage_set_eat_target_hours(p->eat_hours);
    }
}

// ── Eating window ─────────────────────────────────────────────────────────────

void eating_window_restart_fast(void) {
    close_eating_window_into_history();
    uint8_t prog_idx = storage_get_program_id();
    start_fast(prog_idx);
}

void eating_window_end(void) {
    close_eating_window_into_history();
}

// ── OMAD ──────────────────────────────────────────────────────────────────────

void log_meal(void) {
    int32_t prev = storage_get_omad_last_meal_at();
    int32_t now  = time_now();

    if (prev != 0) {
        int32_t  actual_sec   = now - prev;
        int32_t  target_sec   = OMAD_HOURS * 3600;
        int32_t  overtime_sec = actual_sec > target_sec ? actual_sec - target_sec : 0;
        storage_record_fast(actual_sec, overtime_sec);
    }

    storage_set_omad_last_meal_at(now);
}

// ── State + user actions ──────────────────────────────────────────────────────

FastState fasting_state(void) {
    uint8_t prog = storage_get_program_id();
    if (prog == NO_PROGRAM) return FAST_STATE_READY;
    if (program_by_index(prog)->is_omad) {
        return storage_get_omad_last_meal_at() != 0 ? FAST_STATE_OMAD : FAST_STATE_READY;
    }
    if (storage_get_fast_started_at() != 0) return FAST_STATE_ACTIVE;
    if (storage_get_eat_started_at()  != 0) return FAST_STATE_EATING;
    return FAST_STATE_READY;
}

static void schedule_omad_alert(void) {
    notify_schedule(NOTIFY_OMAD_TARGET,
                    storage_get_omad_last_meal_at() + OMAD_HOURS * 3600);
}

static void log_meal_and_schedule_target(void) {
    log_meal();
    schedule_omad_alert();
}

static void schedule_fast_alerts(void) {
    notify_schedule_fast_milestones(storage_get_fast_started_at(),
                                    storage_get_fast_target_hours());
}

static void schedule_eat_alert(void) {
    notify_schedule(NOTIFY_EAT_WINDOW_CLOSED,
                    storage_get_eat_started_at() +
                    (int32_t)storage_get_eat_target_hours() * 3600);
}

bool fasting_select(void) {
    uint8_t prog = storage_get_program_id();
    if (prog == NO_PROGRAM) return false;

    switch (fasting_state()) {
        case FAST_STATE_READY:
            if (program_by_index(prog)->is_omad) {
                log_meal_and_schedule_target();
            } else {
                start_fast(prog);
                schedule_fast_alerts();
            }
            break;

        case FAST_STATE_ACTIVE:
            stop_fast();
            notify_cancel_all_fasting();
            if (storage_get_eat_started_at() != 0) schedule_eat_alert();
            break;

        case FAST_STATE_EATING:
            eating_window_restart_fast();
            schedule_fast_alerts(); // cancels the eat-window alert too
            break;

        case FAST_STATE_OMAD:
            log_meal_and_schedule_target();
            break;
    }
    return true;
}

void fasting_choose_program(uint8_t idx) {
    storage_set_program_id(idx);
    storage_set_fast_started_at(0);
    storage_set_eat_started_at(0);
    storage_set_omad_last_meal_at(0);
    notify_cancel_all_fasting();
}

void fasting_restore_alerts(void) {
    notify_cancel_all_wakeups();
    notify_cancel_all_fasting();
    switch (fasting_state()) {
        case FAST_STATE_ACTIVE: schedule_fast_alerts(); break;
        case FAST_STATE_EATING: schedule_eat_alert();   break;
        case FAST_STATE_OMAD:   schedule_omad_alert();  break;
        case FAST_STATE_READY:  break;
    }
}

void fasting_reset_all(void) {
    notify_cancel_all_fasting();
    storage_reset_all();
}

// ── Stats ─────────────────────────────────────────────────────────────────────

FastStats compute_fast_stats(void) {
    FastStats stats;
    stats.total_fasts           = persist_exists(K_STATS_COUNT)        ? (uint32_t)persist_read_int(K_STATS_COUNT)        : 0;
    uint32_t total_sec          = persist_exists(K_STATS_TOTAL_SEC)    ? (uint32_t)persist_read_int(K_STATS_TOTAL_SEC)    : 0;
    stats.longest_seconds       = persist_exists(K_STATS_LONGEST_SEC)  ? (uint32_t)persist_read_int(K_STATS_LONGEST_SEC)  : 0;
    stats.total_overtime_seconds= persist_exists(K_STATS_OVERTIME_SEC) ? (uint32_t)persist_read_int(K_STATS_OVERTIME_SEC) : 0;
    stats.avg_seconds           = stats.total_fasts > 0 ? total_sec / stats.total_fasts : 0;
    return stats;
}
