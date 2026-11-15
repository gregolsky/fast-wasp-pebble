// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0

#include "glance.h"
#include "fasting.h"
#include "storage.h"

#include <stdio.h>

// Syntax: developer.repebble.com/guides/user-interfaces/appglance-c (predicates unquoted).
// Templates are evaluated by the launcher, so the countdown stays live
// and flips to the "done" text without waking the app.
bool glance_subtitle(char *buf, size_t len) {
    uint8_t prog = storage_get_program_id();
    if (prog == 0xFF) return false;

    if (program_by_index(prog)->is_omad) {
        int32_t meal = storage_get_omad_last_meal_at();
        if (meal == 0) return false;
        snprintf(buf, len, "{time_since(%ld)|format(>=1d:'Over a day since meal','%%aR since meal')}", (long)meal);
        return true;
    }

    int32_t fast = storage_get_fast_started_at();
    if (fast != 0) {
        long end = (long)fast + (long)storage_get_fast_target_hours() * 3600;
        snprintf(buf, len,
                 "{time_until(%ld)|format(>0S:'Fast ends in %%aR','Fast done! Open to eat')}",
                 end);
        return true;
    }

    int32_t eat = storage_get_eat_started_at();
    if (eat != 0) {
        long end = (long)eat + (long)storage_get_eat_target_hours() * 3600;
        snprintf(buf, len,
                 "{time_until(%ld)|format(>0S:'Eat window: %%aR left','Eat window over')}",
                 end);
        return true;
    }
    return false;
}

#ifndef FAST_PEBBLE_HOST_BUILD
#include <pebble.h>

static void reload_cb(AppGlanceReloadSession *session, size_t limit, void *ctx) {
    (void)ctx;
    if (limit < 1) return;
    char sub[150]; // SDK docs: subtitle template max 150 bytes
    if (!glance_subtitle(sub, sizeof(sub))) return; // no slice → launcher default
    AppGlanceSlice slice = {
        .layout = {
            .icon = APP_GLANCE_SLICE_DEFAULT_ICON,
            .subtitle_template_string = sub,
        },
        .expiration_time = APP_GLANCE_SLICE_NO_EXPIRATION,
    };
    AppGlanceResult r = app_glance_add_slice(session, slice);
    if (r != APP_GLANCE_RESULT_SUCCESS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "glance add_slice failed: %d", (int)r);
    }
}

void glance_update(void) {
    app_glance_reload(reload_cb, NULL);
}

#else
void glance_update(void) {}
#endif
