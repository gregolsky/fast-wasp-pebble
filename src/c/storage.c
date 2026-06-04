// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0

#include "storage.h"
#include "platform/persist.h"

// ── Scalar getters/setters ────────────────────────────────────────────────────

uint8_t storage_get_program_id(void) {
    return persist_exists(K_PROGRAM_ID) ? (uint8_t)persist_read_int(K_PROGRAM_ID) : 0xFF;
}
void storage_set_program_id(uint8_t v) { persist_write_int(K_PROGRAM_ID, (int32_t)v); }

int32_t storage_get_fast_started_at(void) {
    return persist_exists(K_FAST_STARTED_AT) ? (int32_t)persist_read_int(K_FAST_STARTED_AT) : 0;
}
void storage_set_fast_started_at(int32_t v) { persist_write_int(K_FAST_STARTED_AT, v); }

uint8_t storage_get_fast_target_hours(void) {
    return persist_exists(K_FAST_TARGET_HOURS) ? (uint8_t)persist_read_int(K_FAST_TARGET_HOURS) : 0;
}
void storage_set_fast_target_hours(uint8_t v) { persist_write_int(K_FAST_TARGET_HOURS, (int32_t)v); }

int32_t storage_get_eat_started_at(void) {
    return persist_exists(K_EAT_STARTED_AT) ? (int32_t)persist_read_int(K_EAT_STARTED_AT) : 0;
}
void storage_set_eat_started_at(int32_t v) { persist_write_int(K_EAT_STARTED_AT, v); }

uint8_t storage_get_eat_target_hours(void) {
    return persist_exists(K_EAT_TARGET_HOURS) ? (uint8_t)persist_read_int(K_EAT_TARGET_HOURS) : 0;
}
void storage_set_eat_target_hours(uint8_t v) { persist_write_int(K_EAT_TARGET_HOURS, (int32_t)v); }

int32_t storage_get_omad_last_meal_at(void) {
    return persist_exists(K_OMAD_LAST_MEAL_AT) ? (int32_t)persist_read_int(K_OMAD_LAST_MEAL_AT) : 0;
}
void storage_set_omad_last_meal_at(int32_t v) { persist_write_int(K_OMAD_LAST_MEAL_AT, v); }

bool storage_get_vibration_on(void) {
    return persist_exists(K_VIBRATION_ON) ? (bool)persist_read_int(K_VIBRATION_ON) : true;
}
void storage_set_vibration_on(bool v) { persist_write_int(K_VIBRATION_ON, (int32_t)v); }

int32_t storage_get_wakeup_id(void) {
    return persist_exists(K_WAKEUP_ID) ? (int32_t)persist_read_int(K_WAKEUP_ID) : -1;
}
void storage_set_wakeup_id(int32_t v) { persist_write_int(K_WAKEUP_ID, v); }

// ── Stats aggregate ───────────────────────────────────────────────────────────

void storage_record_fast(int32_t duration_sec, int32_t overtime_sec) {
    int32_t count    = persist_exists(K_STATS_COUNT)       ? persist_read_int(K_STATS_COUNT)       : 0;
    int32_t total    = persist_exists(K_STATS_TOTAL_SEC)   ? persist_read_int(K_STATS_TOTAL_SEC)   : 0;
    int32_t longest  = persist_exists(K_STATS_LONGEST_SEC) ? persist_read_int(K_STATS_LONGEST_SEC) : 0;
    int32_t overtime = persist_exists(K_STATS_OVERTIME_SEC)? persist_read_int(K_STATS_OVERTIME_SEC): 0;

    count++;
    total    += duration_sec;
    overtime += overtime_sec;
    if (duration_sec > longest) longest = duration_sec;

    persist_write_int(K_STATS_COUNT,        count);
    persist_write_int(K_STATS_TOTAL_SEC,    total);
    persist_write_int(K_STATS_LONGEST_SEC,  longest);
    persist_write_int(K_STATS_OVERTIME_SEC, overtime);
}

// ── Reset ─────────────────────────────────────────────────────────────────────

void storage_reset_all(void) {
    persist_delete(K_PROGRAM_ID);
    persist_delete(K_FAST_STARTED_AT);
    persist_delete(K_FAST_TARGET_HOURS);
    persist_delete(K_EAT_STARTED_AT);
    persist_delete(K_EAT_TARGET_HOURS);
    persist_delete(K_OMAD_LAST_MEAL_AT);
    persist_delete(K_VIBRATION_ON);
    persist_delete(K_WAKEUP_ID);
    persist_delete(K_STATS_COUNT);
    persist_delete(K_STATS_TOTAL_SEC);
    persist_delete(K_STATS_LONGEST_SEC);
    persist_delete(K_STATS_OVERTIME_SEC);
}
