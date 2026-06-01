#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    NOTIFY_FAST_COMPLETE,
    NOTIFY_EAT_WINDOW_CLOSED,
    NOTIFY_OMAD_TARGET,
} NotifyEvent;

// Schedule a wakeup for the given event. Cancels any previously scheduled wakeup.
// target_at: epoch seconds when the event should fire.
void notify_schedule(NotifyEvent event, int32_t target_at);

// Cancel the currently scheduled wakeup (if any).
void notify_cancel(void);

// Call on app launch: returns true if this launch was triggered by a wakeup,
// and populates *event with the event type. Call notify_handle() to act on it.
bool notify_check_wakeup(NotifyEvent *event);

// Vibrate + show appropriate banner for the event (called after wakeup relaunch).
void notify_handle(NotifyEvent event);

// Register a callback invoked when a wakeup fires while the app is already open.
// The callback receives the NotifyEvent reason cast to int32_t.
typedef void (*NotifyInAppCallback)(int32_t event_reason);
void notify_subscribe_inapp(NotifyInAppCallback cb);
