#include "unity.h"
#include "fasting.h"
#include "storage.h"
#include "notify.h"
#include "platform/time.h"
#include "platform/persist.h"

extern int32_t g_now;

#define T0 5000000
#define H  3600

static void restore_setup(bool full_screen) {
    persist_reset_all();
    notify_stub_reset();
    g_now = T0;
    storage_set_full_screen_alerts(full_screen);
}

static void start_16_8(void) {
    storage_set_program_id(2); // 16:8
    fasting_select();
}

static void confirm_pins(void) {
    int32_t at[NOTIFY_EVENT_COUNT];
    notify_pin_times(at);
    for (int e = 0; e < NOTIFY_EVENT_COUNT; e++) storage_set_pin_sent((uint8_t)e, at[e]);
}

// ── Rebuild on launch ─────────────────────────────────────────────────────────

void test_restore_clears_wakeups_orphaned_by_reinstall(void) {
    restore_setup(true);
    start_16_8();
    persist_reset_all(); // uninstall wipes storage; the watch keeps the wakeups
    TEST_ASSERT_TRUE(notify_stub_is_scheduled(NOTIFY_FAST_COMPLETE));

    fasting_restore_alerts();
    TEST_ASSERT_EQUAL_INT(0, notify_stub_scheduled_count());
}

void test_restore_active_fast_keeps_only_future_alerts(void) {
    restore_setup(true);
    start_16_8();
    g_now = T0 + 9 * H;
    fasting_restore_alerts();
    TEST_ASSERT_EQUAL_INT(T0 + 16 * H, notify_stub_target_for(NOTIFY_FAST_COMPLETE));
    TEST_ASSERT_EQUAL_INT(T0 + 12 * H, notify_stub_target_for(NOTIFY_KETOSIS_START));
    TEST_ASSERT_FALSE(notify_stub_is_scheduled(NOTIFY_FAST_HALFWAY));
}

void test_restore_after_goal_does_not_refire_complete(void) {
    restore_setup(true);
    start_16_8();
    g_now = T0 + 17 * H; // goal passed, fast not stopped yet
    fasting_restore_alerts();
    TEST_ASSERT_EQUAL_INT(0, notify_stub_scheduled_count());
    TEST_ASSERT_EQUAL_INT(0, storage_get_alert_at(NOTIFY_FAST_COMPLETE));
}

void test_restore_eating_reschedules_window_close(void) {
    restore_setup(true);
    start_16_8();
    g_now = T0 + 16 * H;
    fasting_select(); // eat
    notify_stub_reset();
    fasting_restore_alerts();
    TEST_ASSERT_EQUAL_INT(T0 + 24 * H, notify_stub_target_for(NOTIFY_EAT_WINDOW_CLOSED));
    TEST_ASSERT_EQUAL_INT(1, notify_stub_scheduled_count());
}

void test_restore_omad_reschedules_meal_alert(void) {
    restore_setup(true);
    storage_set_program_id(5); // OMAD
    fasting_select();          // log first meal
    notify_stub_reset();
    fasting_restore_alerts();
    TEST_ASSERT_EQUAL_INT(T0 + 23 * H, notify_stub_target_for(NOTIFY_OMAD_TARGET));
}

void test_restore_ready_clears_stale_pin_times(void) {
    restore_setup(false);
    storage_set_program_id(2);
    storage_set_alert_at(NOTIFY_FAST_HALFWAY, T0 + H); // left from an older state
    fasting_restore_alerts();
    TEST_ASSERT_EQUAL_INT(0, storage_get_alert_at(NOTIFY_FAST_HALFWAY));
}

void test_restore_moves_pre_upgrade_wakeups_to_pins(void) {
    restore_setup(true);
    start_16_8();
    for (int e = 0; e < NOTIFY_EVENT_COUNT; e++) storage_set_alert_at((uint8_t)e, 0); // as v1.13 stored
    storage_set_full_screen_alerts(false); // v1.14 default
    fasting_restore_alerts();
    TEST_ASSERT_EQUAL_INT(0, notify_stub_scheduled_count());
    TEST_ASSERT_EQUAL_INT(T0 + 16 * H, storage_get_alert_at(NOTIFY_FAST_COMPLETE));
}

void test_schedule_in_past_only_cancels(void) {
    restore_setup(true);
    notify_schedule(NOTIFY_FAST_HALFWAY, T0 + H);
    notify_schedule(NOTIFY_FAST_HALFWAY, T0);
    TEST_ASSERT_FALSE(notify_stub_is_scheduled(NOTIFY_FAST_HALFWAY));
    TEST_ASSERT_EQUAL_INT(0, storage_get_alert_at(NOTIFY_FAST_HALFWAY));
}

// ── Has the phone got the current pins? ───────────────────────────────────────

void test_pins_out_of_sync_until_phone_confirms(void) {
    restore_setup(false);
    TEST_ASSERT_TRUE(notify_pins_in_sync());
    notify_schedule(NOTIFY_FAST_HALFWAY, T0 + 6 * H);
    TEST_ASSERT_FALSE(notify_pins_in_sync());
    confirm_pins();
    TEST_ASSERT_TRUE(notify_pins_in_sync());
}

void test_pins_out_of_sync_after_cancel(void) {
    restore_setup(false);
    notify_schedule(NOTIFY_FAST_HALFWAY, T0 + 6 * H);
    confirm_pins();
    notify_cancel_event(NOTIFY_FAST_HALFWAY);
    TEST_ASSERT_FALSE(notify_pins_in_sync());
}

void test_full_screen_must_withdraw_sent_pins(void) {
    restore_setup(false);
    notify_schedule(NOTIFY_FAST_HALFWAY, T0 + 6 * H);
    confirm_pins();
    notify_set_full_screen(true);
    TEST_ASSERT_FALSE(notify_pins_in_sync());
    confirm_pins();
    TEST_ASSERT_TRUE(notify_pins_in_sync());
}

void test_fired_pin_needs_no_withdrawing(void) {
    restore_setup(false);
    notify_schedule(NOTIFY_FAST_HALFWAY, T0 + 6 * H);
    confirm_pins();
    g_now = T0 + 7 * H;
    notify_cancel_event(NOTIFY_FAST_HALFWAY);
    TEST_ASSERT_TRUE(notify_pins_in_sync());
}
