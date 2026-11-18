#pragma once

#include <stdint.h>
#include <stdbool.h>

// ── Programs ──────────────────────────────────────────────────────────────────

#define NUM_PROGRAMS 6
#define NO_PROGRAM   0xFF
#define OMAD_HOURS   23

typedef struct {
    const char *label;
    const char *sub;
    uint8_t     fast_hours;
    uint8_t     eat_hours;
    bool        is_omad;
} Program;

extern const Program PROGRAMS[NUM_PROGRAMS];

// Index 0..5  (OMAD is index 5)
const Program *program_by_index(uint8_t idx);

// ── State ─────────────────────────────────────────────────────────────────────

typedef enum {
    FAST_STATE_READY,
    FAST_STATE_ACTIVE,
    FAST_STATE_EATING,
    FAST_STATE_OMAD,
} FastState;

// Current state derived from storage (single source for UI and glance).
FastState fasting_state(void);

// ── User actions (state change + alert scheduling) ────────────────────────────

// SELECT on the main screen: start/stop fast, restart from eating, log meal.
// Returns false when nothing happened (no program chosen).
bool fasting_select(void);

// Switch program: clears any running fast/eat/OMAD state and its alerts.
void fasting_choose_program(uint8_t idx);

// Wipe all data and cancel every alert.
void fasting_reset_all(void);

// Rebuild every alert from the saved state (call on launch). Clears wakeups
// the app lost track of, e.g. after an uninstall wiped its storage.
void fasting_restore_alerts(void);

// ── Timer arithmetic ──────────────────────────────────────────────────────────

// Seconds elapsed since started_at (epoch seconds).
int32_t fast_elapsed(int32_t started_at);

// Seconds remaining; negative = overtime.
int32_t fast_remaining(int32_t started_at, uint8_t target_hours);

// Was the target already passed?
bool fast_is_overtime(int32_t started_at, uint8_t target_hours);

// Format absolute value of seconds as "HH:MM:SS" into buf (must be >= 16 bytes).
void fast_format_hms(int32_t total_seconds, char *buf);

// Format absolute value of seconds as "HH:MM" into buf (must be >= 16 bytes).
void fast_format_hm(int32_t total_seconds, char *buf);

// ── Active fast ───────────────────────────────────────────────────────────────

// Start a fast for the given program index. Closes an open eating window first.
void start_fast(uint8_t program_idx);

// Stop the active fast. Records history. Opens eating window if eat_hours > 0.
void stop_fast(void);

// ── Eating window ─────────────────────────────────────────────────────────────

// Close eating window early, then immediately start a new fast (same program).
void eating_window_restart_fast(void);

// Close eating window without starting a new fast.
void eating_window_end(void);

// ── OMAD ──────────────────────────────────────────────────────────────────────

// Log "I ate my meal": closes previous OMAD interval into history, opens new one.
void log_meal(void);

// ── Fasting stats (computed from history ring buffer) ─────────────────────────

typedef struct {
    uint32_t total_fasts;
    uint32_t avg_seconds;
    uint32_t longest_seconds;
    uint32_t total_overtime_seconds;
} FastStats;

FastStats compute_fast_stats(void);
