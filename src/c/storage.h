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
// K_FAST_HISTORY = 7 retired (ring exceeded 256-byte per-key limit)
#define K_VIBRATION_ON       8u
#define K_WAKEUP_ID          9u
#define K_STATS_COUNT        10u
#define K_STATS_TOTAL_SEC    11u
#define K_STATS_LONGEST_SEC  12u
#define K_STATS_OVERTIME_SEC 13u

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

// ── Stats aggregate ───────────────────────────────────────────────────────────

// Fold one completed fast into the persisted aggregates.
void storage_record_fast(int32_t duration_sec, int32_t overtime_sec);

// Wipe everything.
void storage_reset_all(void);
