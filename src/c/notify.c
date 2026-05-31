// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0

#include "notify.h"
#include "storage.h"

#ifndef FAST_PEBBLE_HOST_BUILD
#include <pebble.h>

void notify_schedule(NotifyEvent event, int32_t target_at) {
    notify_cancel();

    WakeupId id = wakeup_schedule((time_t)target_at, (int32_t)event, true);
    if (id >= 0) {
        storage_set_wakeup_id((int32_t)id);
    }
}

void notify_cancel(void) {
    int32_t id = storage_get_wakeup_id();
    if (id >= 0) {
        wakeup_cancel((WakeupId)id);
        storage_set_wakeup_id(-1);
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
    if (storage_get_vibration_on()) {
        switch (event) {
            case NOTIFY_FAST_COMPLETE:
                vibes_long_pulse();
                break;
            case NOTIFY_EAT_WINDOW_CLOSED:
                vibes_double_pulse();
                break;
            case NOTIFY_OMAD_TARGET:
                vibes_long_pulse();
                break;
        }
    }
    // Banner text is rendered by the calling UI layer (main.c).
}

#else
// ── Host stubs (unit-test build) ──────────────────────────────────────────────

static int32_t s_last_wakeup_target = 0;
static int32_t s_last_wakeup_event  = -1;
static int     s_cancel_count       = 0;

int32_t notify_stub_last_target(void) { return s_last_wakeup_target; }
int32_t notify_stub_last_event(void)  { return s_last_wakeup_event;  }
int     notify_stub_cancel_count(void){ return s_cancel_count;        }
void    notify_stub_reset(void)       { s_last_wakeup_target = 0; s_last_wakeup_event = -1; s_cancel_count = 0; }

void notify_schedule(NotifyEvent event, int32_t target_at) {
    notify_cancel();
    s_last_wakeup_target = target_at;
    s_last_wakeup_event  = (int32_t)event;
    storage_set_wakeup_id(42); // sentinel
}

void notify_cancel(void) {
    int32_t id = storage_get_wakeup_id();
    if (id >= 0) {
        s_cancel_count++;
        storage_set_wakeup_id(-1);
    }
}

bool notify_check_wakeup(NotifyEvent *event) { (void)event; return false; }
void notify_handle(NotifyEvent event)        { (void)event; }

#endif
