// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0

#include "pins.h"
#include "notify.h"
#include "storage.h"

#include <pebble.h>
#include "message_keys.auto.h"

// The phone (src/pkjs/index.js) turns the alert times into timeline pins.
// The whole table is sent each time, so a lost message is fixed by the next.

#define RETRY_MS     2000
#define MAX_RETRIES  5

static bool      s_js_ready;
static bool      s_sending;
static bool      s_pending;
static int       s_retries;
static AppTimer *s_timer;

static void kick(uint32_t ms);

static void send_now(void *ctx) {
    (void)ctx;
    s_timer = NULL;
    if (!s_js_ready || s_sending || !s_pending) return;

    bool pins_on = !storage_get_full_screen_alerts();
    int32_t at[NOTIFY_EVENT_COUNT];
    for (int e = 0; e < NOTIFY_EVENT_COUNT; e++) {
        at[e] = pins_on ? storage_get_alert_at((uint8_t)e) : 0;
    }

    DictionaryIterator *it;
    if (app_message_outbox_begin(&it) != APP_MSG_OK) { kick(RETRY_MS); return; }
    dict_write_data(it, MESSAGE_KEY_PINS, (const uint8_t *)at, sizeof(at)); // little-endian on ARM
    if (app_message_outbox_send() != APP_MSG_OK) { kick(RETRY_MS); return; }
    s_sending = true;
    s_pending = false;
}

static void kick(uint32_t ms) {
    if (!s_timer) s_timer = app_timer_register(ms, send_now, NULL);
}

static void on_sent(DictionaryIterator *it, void *ctx) {
    (void)it; (void)ctx;
    s_sending = false;
    s_retries = 0;
    APP_LOG(APP_LOG_LEVEL_INFO, "pins-sent");
    if (s_pending) kick(0);
}

static void on_failed(DictionaryIterator *it, AppMessageResult reason, void *ctx) {
    (void)it; (void)ctx;
    s_sending = false;
    s_pending = true;
    APP_LOG(APP_LOG_LEVEL_WARNING, "pins-send-failed:%d", (int)reason);
    // Phone gone: give up until the next launch, which resyncs everything.
    if (++s_retries <= MAX_RETRIES) kick(RETRY_MS);
}

static void on_received(DictionaryIterator *it, void *ctx) {
    (void)ctx;
    if (dict_find(it, MESSAGE_KEY_READY)) {
        s_js_ready = true;
        pins_sync(); // resync on every launch
    }
}

void pins_init(void) {
    app_message_register_inbox_received(on_received);
    app_message_register_outbox_sent(on_sent);
    app_message_register_outbox_failed(on_failed);
    app_message_open(64, 64);
}

void pins_sync(void) {
    s_pending = true;
    kick(50); // coalesce a burst of schedule/cancel calls into one message
}
