// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0

#include "storage.h"
#include "platform/persist.h"

#include <string.h>

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

// ── History ring ──────────────────────────────────────────────────────────────

static void load_ring(HistoryRing *ring) {
    int result = persist_read_data(K_FAST_HISTORY, ring, sizeof(HistoryRing));
    if (result != (int)sizeof(HistoryRing)) {
        memset(ring, 0, sizeof(HistoryRing));
    }
}

static void save_ring(const HistoryRing *ring) {
    persist_write_data(K_FAST_HISTORY, ring, sizeof(HistoryRing));
}

void storage_push_fast_history(const HistoryEntry *e) {
    HistoryRing ring;
    load_ring(&ring);

    ring.entries[ring.head] = *e;
    ring.head = (ring.head + 1) % FAST_HISTORY_CAPACITY;
    if (ring.count < FAST_HISTORY_CAPACITY) ring.count++;

    save_ring(&ring);

#ifndef FAST_PEBBLE_HOST_BUILD
    APP_LOG(APP_LOG_LEVEL_DEBUG, "ring-count:%u ring-size:%d",
            (unsigned)ring.count, persist_get_size(K_FAST_HISTORY));
#endif
}

uint8_t storage_read_fast_history(HistoryEntry *out, uint8_t max_count) {
    HistoryRing ring;
    load_ring(&ring);

    uint8_t n = ring.count < max_count ? ring.count : max_count;
    // head points to the slot that will be written next (= oldest if full, else unused).
    // chronological start = (head - count + CAPACITY) % CAPACITY when full,
    // or simply 0..count-1 when not full.
    uint8_t start = ring.count < FAST_HISTORY_CAPACITY
                    ? 0
                    : ring.head; // head is oldest when ring is full

    for (uint8_t i = 0; i < n; i++) {
        uint8_t idx = (start + i) % FAST_HISTORY_CAPACITY;
        out[i] = ring.entries[idx];
    }
    return n;
}

int storage_history_bytes(void) {
    return persist_get_size(K_FAST_HISTORY);
}

// ── Reset ─────────────────────────────────────────────────────────────────────

void storage_reset_all(void) {
    persist_delete(K_PROGRAM_ID);
    persist_delete(K_FAST_STARTED_AT);
    persist_delete(K_FAST_TARGET_HOURS);
    persist_delete(K_EAT_STARTED_AT);
    persist_delete(K_EAT_TARGET_HOURS);
    persist_delete(K_OMAD_LAST_MEAL_AT);
    persist_delete(K_FAST_HISTORY);
    persist_delete(K_VIBRATION_ON);
    persist_delete(K_WAKEUP_ID);
}
