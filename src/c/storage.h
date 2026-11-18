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
// K_WAKEUP_ID = 9 retired — per-event wakeup IDs now use K_WAKEUP_ID_BASE + event
#define K_STATS_COUNT        10u
#define K_STATS_TOTAL_SEC    11u
#define K_STATS_LONGEST_SEC  12u
#define K_STATS_OVERTIME_SEC 13u
// Per-event wakeup IDs: keys K_WAKEUP_ID_BASE+0 .. K_WAKEUP_ID_BASE+6 (events 0..6)
#define K_WAKEUP_ID_BASE     14u
#define K_MILESTONES_ON      21u
// Per-event alert times (0 = none): keys K_ALERT_AT_BASE+0 .. +6
#define K_ALERT_AT_BASE      22u
#define K_FULL_SCREEN_ALERTS 29u

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

bool     storage_get_milestones_on(void);
void     storage_set_milestones_on(bool v);

// Alert style: false = timeline pins (default), true = full-screen wakeup alerts.
bool     storage_get_full_screen_alerts(void);
void     storage_set_full_screen_alerts(bool v);

// Per-event alert time, whatever the style (managed by notify.c).
int32_t  storage_get_alert_at(uint8_t event);
void     storage_set_alert_at(uint8_t event, int32_t at);

// Per-event wakeup ID slots (managed by notify.c).
int32_t  storage_get_wakeup_id_for_event(uint8_t event);
void     storage_set_wakeup_id_for_event(uint8_t event, int32_t id);

// ── Stats aggregate ───────────────────────────────────────────────────────────

// Fold one completed fast into the persisted aggregates.
void storage_record_fast(int32_t duration_sec, int32_t overtime_sec);

// Wipe stats only (count, avg, longest, overtime).
void storage_reset_stats(void);

// Wipe everything.
void storage_reset_all(void);
