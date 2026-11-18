#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    NOTIFY_FAST_COMPLETE     = 0,
    NOTIFY_EAT_WINDOW_CLOSED = 1,
    NOTIFY_OMAD_TARGET       = 2,
    NOTIFY_FAST_HALFWAY      = 3,
    NOTIFY_FAST_1H_LEFT      = 4,
    NOTIFY_KETOSIS_START     = 5,
    NOTIFY_AUTOPHAGY_START   = 6,
    NOTIFY_EVENT_COUNT       = 7,
} NotifyEvent;

#define KETOSIS_OFFSET_S     (12 * 3600)
#define AUTOPHAGY_OFFSET_S   (16 * 3600)
#define ONE_HOUR_S           3600
#define COLLISION_THRESHOLD_S 60
#define COLLISION_SHIFT_S    (30 * 60)

// Schedule the alert for the given event, as a timeline pin or a full-screen
// wakeup depending on the style setting. Replaces any earlier one for the event.
// A time already past only cancels: alerts never fire late.
void notify_schedule(NotifyEvent event, int32_t target_at);

// Cancel the alert for the given event (if any), in either style.
void notify_cancel_event(NotifyEvent event);

// Switch alert style (Settings): moves pending alerts between full-screen
// wakeups and timeline pins.
void notify_set_full_screen(bool on);

// Cancel all fasting-related alerts (call on stop/reset).
void notify_cancel_all_fasting(void);

// Cancel every wakeup this app owns, including ones it lost track of: the
// watch keeps an app's wakeups when it is uninstalled but wipes its storage.
void notify_cancel_all_wakeups(void);

// The pin times the phone should hold: future alert times, or all zeros in
// the full-screen style.
void notify_pin_times(int32_t at[NOTIFY_EVENT_COUNT]);

// True when the phone has confirmed the current pin times.
bool notify_pins_in_sync(void);

// Schedule FAST_COMPLETE + all applicable fast milestones based on start time
// and target duration. Milestones are skipped if K_MILESTONES_ON is false.
void notify_schedule_fast_milestones(int32_t started_at, uint8_t target_hours);

// Call on app launch: returns true if this launch was triggered by a wakeup,
// and populates *event with the event type. Call notify_handle() to act on it.
bool notify_check_wakeup(NotifyEvent *event);

// Vibrate for the event (called after wakeup relaunch). Gated by vibration toggle.
void notify_handle(NotifyEvent event);

// Register a callback invoked when a wakeup fires while the app is already open.
typedef void (*NotifyInAppCallback)(int32_t event_reason);
void notify_subscribe_inapp(NotifyInAppCallback cb);

// ── Unit-test introspection (host build only) ─────────────────────────────────
#ifdef FAST_PEBBLE_HOST_BUILD
int     notify_stub_scheduled_count(void);
bool    notify_stub_is_scheduled(NotifyEvent e);
int32_t notify_stub_target_for(NotifyEvent e);
int32_t notify_stub_last_target(void);
int32_t notify_stub_last_event(void);
int     notify_stub_cancel_count(void);
void    notify_stub_reset(void);
#endif
