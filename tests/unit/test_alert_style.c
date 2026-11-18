#include "unity.h"
#include "storage.h"
#include "notify.h"
#include "platform/time.h"
#include "platform/persist.h"

extern int32_t g_now;
extern int     g_pins_sync_count;

#define T0 5000000
#define H  3600

static void style_setup(void) {
    persist_reset_all();
    g_now = T0;
    notify_stub_reset();
    g_pins_sync_count = 0;
}

void test_timeline_is_default_style(void) {
    style_setup();
    TEST_ASSERT_FALSE(storage_get_full_screen_alerts());
}

void test_timeline_schedule_syncs_pins_not_wakeups(void) {
    style_setup();
    notify_schedule(NOTIFY_FAST_HALFWAY, T0 + 6 * H);
    TEST_ASSERT_EQUAL_INT(T0 + 6 * H, storage_get_alert_at(NOTIFY_FAST_HALFWAY));
    TEST_ASSERT_EQUAL_INT(0, notify_stub_scheduled_count());
    TEST_ASSERT_TRUE(g_pins_sync_count > 0);
}

void test_timeline_cancel_clears_alert_and_syncs(void) {
    style_setup();
    notify_schedule(NOTIFY_FAST_HALFWAY, T0 + 6 * H);
    g_pins_sync_count = 0;
    notify_cancel_event(NOTIFY_FAST_HALFWAY);
    TEST_ASSERT_EQUAL_INT(0, storage_get_alert_at(NOTIFY_FAST_HALFWAY));
    TEST_ASSERT_EQUAL_INT(1, g_pins_sync_count);
}

void test_cancel_unscheduled_does_not_sync(void) {
    style_setup();
    notify_cancel_all_fasting();
    TEST_ASSERT_EQUAL_INT(0, g_pins_sync_count);
}

void test_full_screen_schedule_uses_wakeup(void) {
    style_setup();
    storage_set_full_screen_alerts(true);
    notify_schedule(NOTIFY_FAST_COMPLETE, T0 + 12 * H);
    TEST_ASSERT_TRUE(notify_stub_is_scheduled(NOTIFY_FAST_COMPLETE));
    TEST_ASSERT_EQUAL_INT(T0 + 12 * H, storage_get_alert_at(NOTIFY_FAST_COMPLETE));
    TEST_ASSERT_EQUAL_INT(0, g_pins_sync_count);
}

void test_switch_to_full_screen_moves_only_future_alerts(void) {
    style_setup();
    notify_schedule(NOTIFY_FAST_HALFWAY,  T0 + 6 * H);
    notify_schedule(NOTIFY_FAST_COMPLETE, T0 + 12 * H);
    g_now = T0 + 7 * H; // halfway already passed
    notify_set_full_screen(true);
    TEST_ASSERT_FALSE(notify_stub_is_scheduled(NOTIFY_FAST_HALFWAY));
    TEST_ASSERT_EQUAL_INT(T0 + 12 * H, notify_stub_target_for(NOTIFY_FAST_COMPLETE));
    TEST_ASSERT_TRUE(g_pins_sync_count > 0); // withdraws the pending pins
}

void test_switch_back_to_timeline_cancels_wakeups_keeps_times(void) {
    style_setup();
    storage_set_full_screen_alerts(true);
    notify_schedule(NOTIFY_FAST_COMPLETE, T0 + 12 * H);
    notify_set_full_screen(false);
    TEST_ASSERT_EQUAL_INT(0, notify_stub_scheduled_count());
    TEST_ASSERT_EQUAL_INT(-1, storage_get_wakeup_id_for_event(NOTIFY_FAST_COMPLETE));
    TEST_ASSERT_EQUAL_INT(T0 + 12 * H, storage_get_alert_at(NOTIFY_FAST_COMPLETE));
    TEST_ASSERT_TRUE(g_pins_sync_count > 0);
}

void test_reset_all_restores_timeline_style(void) {
    style_setup();
    storage_set_full_screen_alerts(true);
    storage_set_alert_at(NOTIFY_FAST_COMPLETE, T0);
    storage_reset_all();
    TEST_ASSERT_FALSE(storage_get_full_screen_alerts());
    TEST_ASSERT_EQUAL_INT(0, storage_get_alert_at(NOTIFY_FAST_COMPLETE));
}
