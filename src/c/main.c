// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0

#include <pebble.h>

#include "version.h"
#include "storage.h"
#include "notify.h"
#include "ui_fast.h"
#include "ui_picker.h"
#include "fasting.h"

static bool        s_launch_wakeup = false;
static NotifyEvent s_launch_event;

static void handle_wakeup_on_launch(void) {
    if (!notify_check_wakeup(&s_launch_event)) return;
    s_launch_wakeup = true;
    notify_handle(s_launch_event);
}

static void inapp_wakeup_cb(int32_t reason) {
    NotifyEvent event = (NotifyEvent)reason;
    notify_handle(event);
    ui_fast_show_banner(event);
}

static void on_first_program_selected(uint8_t idx) {
    storage_set_program_id(idx);
    ui_fast_push();
}

static void app_init(void) {
    APP_LOG(APP_LOG_LEVEL_INFO, "app-version:" APP_VERSION);
    handle_wakeup_on_launch();

    if (storage_get_program_id() == 0xFF) {
        APP_LOG(APP_LOG_LEVEL_INFO, "state-picker");
        ui_picker_push(on_first_program_selected);
    } else {
        ui_fast_push();
        if (s_launch_wakeup) {
            ui_fast_show_banner(s_launch_event);
        }
    }

    notify_subscribe_inapp(inapp_wakeup_cb);
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
