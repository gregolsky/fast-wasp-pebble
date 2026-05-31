#pragma once

#include <stdint.h>
#include <stdbool.h>

// ── Persistent storage keys ───────────────────────────────────────────────────

#define K_PROGRAM_ID         1u
#define K_FAST_STARTED_AT    2u
#define K_FAST_TARGET_HOURS  3u
#define K_EAT_STARTED_AT     4u
#define K_EAT_TARGET_HOURS   5u
#define K_OMAD_LAST_MEAL_AT  6u
#define K_FAST_HISTORY       7u
#define K_VIBRATION_ON       8u
#define K_WAKEUP_ID          9u

// ── Fast-history ring buffer ──────────────────────────────────────────────────

#define FAST_HISTORY_CAPACITY 64u

typedef struct __attribute__((packed)) {
    int32_t started_at;    // epoch seconds
    int32_t duration_sec;
    int32_t overtime_sec;
} HistoryEntry;            // 12 bytes per entry => 768 bytes total

typedef struct __attribute__((packed)) {
    uint8_t      head;                              // next write index (wraps)
    uint8_t      count;                             // 0..64
    HistoryEntry entries[FAST_HISTORY_CAPACITY];
} HistoryRing;

// ── Getters / setters ─────────────────────────────────────────────────────────

uint8_t  storage_get_program_id(void);
void     storage_set_program_id(uint8_t v);

int32_t  storage_get_fast_started_at(void);
void     storage_set_fast_started_at(int32_t v);

uint8_t  storage_get_fast_target_hours(void);
void     storage_set_fast_target_hours(uint8_t v);

int32_t  storage_get_eat_started_at(void);
void     storage_set_eat_started_at(int32_t v);

uint8_t  storage_get_eat_target_hours(void);
void     storage_set_eat_target_hours(uint8_t v);

int32_t  storage_get_omad_last_meal_at(void);
void     storage_set_omad_last_meal_at(int32_t v);

bool     storage_get_vibration_on(void);
void     storage_set_vibration_on(bool v);

int32_t  storage_get_wakeup_id(void);
void     storage_set_wakeup_id(int32_t v);

// ── History ring ──────────────────────────────────────────────────────────────

// Push one entry (evicts oldest when full).
void    storage_push_fast_history(const HistoryEntry *e);

// Read up to max_count entries in chronological order. Returns actual count.
uint8_t storage_read_fast_history(HistoryEntry *out, uint8_t max_count);

// Wipe everything.
void    storage_reset_all(void);

// Total bytes used by the history blob.
int     storage_history_bytes(void);
