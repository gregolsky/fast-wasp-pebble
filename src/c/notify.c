// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0

#include "notify.h"
#include "storage.h"
#include "platform/time.h"
#include "pins.h"

// ── Helpers (shared between device and host) ──────────────────────────────────

static int32_t i32abs(int32_t v) { return v < 0 ? -v : v; }

// Full-screen backend, defined per build below.
static void wakeup_add(NotifyEvent event, int32_t target_at);
static void wakeup_remove(NotifyEvent event);

// ── Milestone scheduling logic (shared, pure C) ───────────────────────────────

void notify_schedule_fast_milestones(int32_t started_at, uint8_t target_hours) {
    int32_t now      = time_now();
    int32_t goal_sec = (int32_t)target_hours * 3600;
    int32_t goal_at  = started_at + goal_sec;

    notify_cancel_all_fasting();
    notify_schedule(NOTIFY_FAST_COMPLETE, goal_at);

    if (!storage_get_milestones_on()) return;

    // 1h-left: goal-relative, no shift
    int32_t h1l = goal_at - ONE_HOUR_S;
    if (h1l > now) {
        notify_schedule(NOTIFY_FAST_1H_LEFT, h1l);
    } else {
        h1l = -1; // not scheduled, don't treat as anchor
    }

    // Halfway: non-goal-relative; shift 30 min earlier if it collides with 1h-left
    int32_t hal = started_at + goal_sec / 2;
    if (h1l != -1 && i32abs(hal - h1l) < COLLISION_THRESHOLD_S) {
        hal -= COLLISION_SHIFT_S;
    }
    if (hal > now) {
        notify_schedule(NOTIFY_FAST_HALFWAY, hal);
    } else {
        hal = -1;
    }

    // Ketosis: non-goal-relative; shift earlier if within threshold of 1h-left
    int32_t ket = started_at + KETOSIS_OFFSET_S;
    if (ket < goal_at) {
        if (h1l != -1 && i32abs(ket - h1l) < COLLISION_THRESHOLD_S) {
            ket -= COLLISION_SHIFT_S;
        }
        if (ket > now && ket < goal_at) {
            notify_schedule(NOTIFY_KETOSIS_START, ket);
        } else {
            ket = -1;
        }
    } else {
        ket = -1;
    }

    // Autophagy: non-goal-relative; if it hits the goal, shift 30 min earlier.
    // If it's already past the goal (not reached within program), drop it.
    int32_t aut = started_at + AUTOPHAGY_OFFSET_S;
    if (aut >= goal_at) {
        // Shift earlier and check it doesn't land on another anchor
        aut -= COLLISION_SHIFT_S;
        if ((ket != -1 && i32abs(aut - ket) < COLLISION_THRESHOLD_S) ||
            (h1l != -1 && i32abs(aut - h1l) < COLLISION_THRESHOLD_S) ||
            aut <= now || aut >= goal_at) {
            aut = -1;
        }
    } else {
        // Falls within fast — check collision with ketosis or 1h-left
        if ((ket != -1 && i32abs(aut - ket) < COLLISION_THRESHOLD_S) ||
            (h1l != -1 && i32abs(aut - h1l) < COLLISION_THRESHOLD_S)) {
            aut -= COLLISION_SHIFT_S;
        }
        if (aut <= now || aut >= goal_at) aut = -1;
    }
    if (aut != -1) {
        notify_schedule(NOTIFY_AUTOPHAGY_START, aut);
    }
}

void notify_cancel_all_fasting(void) {
    for (int e = 0; e < NOTIFY_EVENT_COUNT; e++) {
        notify_cancel_event((NotifyEvent)e);
    }
}

// ── Device implementation ─────────────────────────────────────────────────────

#ifndef FAST_PEBBLE_HOST_BUILD
#include <pebble.h>

static void wakeup_add(NotifyEvent event, int32_t target_at) {
    WakeupId id = wakeup_schedule((time_t)target_at, (int32_t)event, true);
    if (id < 0) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "wakeup_schedule failed id=%ld target=%ld event=%d",
                (long)id, (long)target_at, (int)event);
        return;
    }
    storage_set_wakeup_id_for_event((uint8_t)event, (int32_t)id);
}

static void wakeup_remove(NotifyEvent event) {
    int32_t id = storage_get_wakeup_id_for_event((uint8_t)event);
    if (id >= 0) {
        wakeup_cancel((WakeupId)id);
        storage_set_wakeup_id_for_event((uint8_t)event, -1);
    }
}

bool notify_check_wakeup(NotifyEvent *event) {
    WakeupId id;
    int32_t  reason;
    if (launch_reason() == APP_LAUNCH_WAKEUP) {
        wakeup_get_launch_event(&id, &reason);
        *event = (NotifyEvent)reason;
        return true;
    }
    return false;
}

void notify_handle(NotifyEvent event) {
    if (!storage_get_vibration_on()) return;
    switch (event) {
        case NOTIFY_FAST_COMPLETE:
            vibes_long_pulse();
            break;
        case NOTIFY_EAT_WINDOW_CLOSED:
            vibes_double_pulse();
            break;
        case NOTIFY_OMAD_TARGET:
        case NOTIFY_FAST_HALFWAY:
        case NOTIFY_FAST_1H_LEFT:
        case NOTIFY_KETOSIS_START:
        case NOTIFY_AUTOPHAGY_START:
            vibes_long_pulse();
            break;
        default:
            break;
    }
}

static NotifyInAppCallback s_inapp_cb;

static void wakeup_adapter(WakeupId id, int32_t reason) {
    (void)id;
    if (s_inapp_cb) s_inapp_cb(reason);
}

void notify_subscribe_inapp(NotifyInAppCallback cb) {
    s_inapp_cb = cb;
    wakeup_service_subscribe(wakeup_adapter);
}

// ── Host stubs (unit-test build) ──────────────────────────────────────────────

#else

#define NOTIFY_STUB_MAX  8

static int32_t s_sched_events[NOTIFY_STUB_MAX];
static int32_t s_sched_targets[NOTIFY_STUB_MAX];
static int     s_sched_count;
static int     s_cancel_count;

int notify_stub_scheduled_count(void) { return s_sched_count; }

int32_t notify_stub_target_for(NotifyEvent e) {
    for (int i = 0; i < s_sched_count; i++)
        if (s_sched_events[i] == (int32_t)e) return s_sched_targets[i];
    return -1;
}

bool notify_stub_is_scheduled(NotifyEvent e) {
    return notify_stub_target_for(e) >= 0;
}

int32_t notify_stub_last_target(void) {
    return s_sched_count > 0 ? s_sched_targets[s_sched_count - 1] : 0;
}

int32_t notify_stub_last_event(void) {
    return s_sched_count > 0 ? s_sched_events[s_sched_count - 1] : -1;
}

int notify_stub_cancel_count(void) { return s_cancel_count; }

void notify_stub_reset(void) {
    for (int i = 0; i < NOTIFY_STUB_MAX; i++) {
        s_sched_events[i]  = -1;
        s_sched_targets[i] = 0;
    }
    s_sched_count  = 0;
    s_cancel_count = 0;
}

static void wakeup_remove(NotifyEvent event) {
    int32_t id = storage_get_wakeup_id_for_event((uint8_t)event);
    if (id < 0) return;
    s_cancel_count++;
    storage_set_wakeup_id_for_event((uint8_t)event, -1);
    for (int i = 0; i < s_sched_count; i++) {
        if (s_sched_events[i] == (int32_t)event) {
            for (int j = i; j < s_sched_count - 1; j++) {
                s_sched_events[j]  = s_sched_events[j + 1];
                s_sched_targets[j] = s_sched_targets[j + 1];
            }
            s_sched_count--;
            break;
        }
    }
}

static void wakeup_add(NotifyEvent event, int32_t target_at) {
    if (s_sched_count < NOTIFY_STUB_MAX) {
        s_sched_events[s_sched_count]  = (int32_t)event;
        s_sched_targets[s_sched_count] = target_at;
        s_sched_count++;
    }
    storage_set_wakeup_id_for_event((uint8_t)event, 42); // sentinel
}

bool notify_check_wakeup(NotifyEvent *event) { (void)event; return false; }
void notify_handle(NotifyEvent event)        { (void)event; }
void notify_subscribe_inapp(NotifyInAppCallback cb) { (void)cb; }

#endif

// ── Alert-style routing (shared) ───────────────────────────────────────────────

// The alert time is kept whatever the style, so switching style can move
// pending alerts between wakeups and timeline pins.
void notify_schedule(NotifyEvent event, int32_t target_at) {
    notify_cancel_event(event);
    storage_set_alert_at((uint8_t)event, target_at);
    if (storage_get_full_screen_alerts()) {
        wakeup_add(event, target_at);
    } else {
        pins_sync();
    }
}

void notify_cancel_event(NotifyEvent event) {
    wakeup_remove(event);
    if (storage_get_alert_at((uint8_t)event)) {
        storage_set_alert_at((uint8_t)event, 0);
        pins_sync();
    }
}

void notify_set_full_screen(bool on) {
    storage_set_full_screen_alerts(on);
    int32_t now = time_now();
    for (int e = 0; e < NOTIFY_EVENT_COUNT; e++) {
        int32_t at = storage_get_alert_at((uint8_t)e);
        // No time stored: a wakeup scheduled before v1.14 kept times; leave it be.
        if (!at) continue;
        wakeup_remove((NotifyEvent)e);
        if (on && at > now) wakeup_add((NotifyEvent)e, at);
    }
    pins_sync(); // sends zeros when full screen, withdrawing pending pins
}
