// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0

#include <pebble.h>

#include "storage.h"
#include "notify.h"
#include "ui_fast.h"
#include "ui_picker.h"
#include "fasting.h"

static void handle_wakeup_on_launch(void) {
    NotifyEvent event;
    if (!notify_check_wakeup(&event)) return;

    // Transition state before showing UI.
    switch (event) {
        case NOTIFY_FAST_COMPLETE:
            stop_fast();
            // Eating window (if any) is now open; schedule its wakeup.
            if (storage_get_eat_started_at() != 0) {
                int32_t es = storage_get_eat_started_at();
                uint8_t eh = storage_get_eat_target_hours();
                notify_schedule(NOTIFY_EAT_WINDOW_CLOSED, es + (int32_t)eh * 3600);
            }
            break;

        case NOTIFY_EAT_WINDOW_CLOSED:
            eating_window_end();
            break;

        case NOTIFY_OMAD_TARGET:
            // OMAD stays open; wakeup was informational only.
            break;
    }

    notify_handle(event);
}

static void on_first_program_selected(uint8_t idx) {
    storage_set_program_id(idx);
    ui_fast_push();
}

static void app_init(void) {
    handle_wakeup_on_launch();

    if (storage_get_program_id() == 0xFF) {
        APP_LOG(APP_LOG_LEVEL_INFO, "state-picker");
        ui_picker_push(on_first_program_selected);
    } else {
        ui_fast_push();
    }
}

static void app_deinit(void) {
    // Nothing to clean up — Pebble reclaims all heap on exit.
}

int main(void) {
    app_init();
    app_event_loop();
    app_deinit();
    return 0;
}
