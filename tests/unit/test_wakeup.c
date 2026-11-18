#include "unity.h"
#include "fasting.h"
#include "storage.h"
#include "notify.h"
#include "platform/time.h"
#include "platform/persist.h"

extern int32_t g_now;
extern PersistSlot g_persist[PERSIST_MAX_SLOTS];

#define T0 5000000
#define H (3600)

static void wakeup_setup(void) {
    persist_reset_all();
    g_now = T0;
    notify_stub_reset();
    storage_set_full_screen_alerts(true); // the stub records wakeups
}

// ── Basic schedule / cancel ───────────────────────────────────────────────────

void test_wakeup_target_for_16h_fast(void) {
    wakeup_setup();
    start_fast(2); // 16:8
    int32_t started  = storage_get_fast_started_at();
    uint8_t hours    = storage_get_fast_target_hours();
    int32_t expected = started + (int32_t)hours * H;

    notify_schedule(NOTIFY_FAST_COMPLETE, expected);
    TEST_ASSERT_EQUAL_INT(expected, notify_stub_last_target());
    TEST_ASSERT_EQUAL_INT(NOTIFY_FAST_COMPLETE, notify_stub_last_event());
}

void test_cancel_event_clears_storage(void) {
    wakeup_setup();
    notify_schedule(NOTIFY_FAST_COMPLETE, T0 + 16 * H);
    TEST_ASSERT_EQUAL_INT(42, storage_get_wakeup_id_for_event((uint8_t)NOTIFY_FAST_COMPLETE));

    notify_cancel_event(NOTIFY_FAST_COMPLETE);
    TEST_ASSERT_EQUAL_INT(-1, storage_get_wakeup_id_for_event((uint8_t)NOTIFY_FAST_COMPLETE));
    TEST_ASSERT_EQUAL_INT(1,  notify_stub_cancel_count());
}

void test_schedule_same_event_cancels_previous(void) {
    wakeup_setup();
    notify_schedule(NOTIFY_FAST_COMPLETE, T0 + 16 * H);
    notify_schedule(NOTIFY_FAST_COMPLETE, T0 + 17 * H); // re-schedule same event
    TEST_ASSERT_EQUAL_INT(1, notify_stub_cancel_count());  // first slot cancelled
    TEST_ASSERT_EQUAL_INT(T0 + 17 * H, notify_stub_last_target());
}

void test_schedule_different_events_independent(void) {
    wakeup_setup();
    notify_schedule(NOTIFY_FAST_COMPLETE,     T0 + 16 * H);
    notify_schedule(NOTIFY_EAT_WINDOW_CLOSED, T0 + 24 * H);
    TEST_ASSERT_EQUAL_INT(0, notify_stub_cancel_count()); // different slots, no cancel
    TEST_ASSERT_EQUAL_INT(2, notify_stub_scheduled_count());
}

void test_no_cancel_when_no_wakeup(void) {
    wakeup_setup();
    notify_cancel_event(NOTIFY_FAST_COMPLETE);
    TEST_ASSERT_EQUAL_INT(0, notify_stub_cancel_count());
}

void test_cancel_all_fasting_cancels_every_event(void) {
    wakeup_setup();
    notify_schedule(NOTIFY_FAST_COMPLETE,     T0 + 16 * H);
    notify_schedule(NOTIFY_FAST_1H_LEFT,      T0 + 15 * H);
    notify_schedule(NOTIFY_FAST_HALFWAY,      T0 +  8 * H);
    notify_schedule(NOTIFY_KETOSIS_START,     T0 + 12 * H);
    int before_cancel = notify_stub_cancel_count();
    notify_cancel_all_fasting();
    TEST_ASSERT_EQUAL_INT(before_cancel + 4, notify_stub_cancel_count());
    TEST_ASSERT_EQUAL_INT(0, notify_stub_scheduled_count());
}

// ── Milestone scheduling ──────────────────────────────────────────────────────

void test_milestones_16h_fast_schedules_all(void) {
    wakeup_setup();
    storage_set_milestones_on(true);
    // started_at = T0, goal = 16h
    notify_schedule_fast_milestones(T0, 16);

    TEST_ASSERT_TRUE(notify_stub_is_scheduled(NOTIFY_FAST_COMPLETE));
    TEST_ASSERT_TRUE(notify_stub_is_scheduled(NOTIFY_FAST_1H_LEFT));
    TEST_ASSERT_TRUE(notify_stub_is_scheduled(NOTIFY_FAST_HALFWAY));
    TEST_ASSERT_TRUE(notify_stub_is_scheduled(NOTIFY_KETOSIS_START));
    TEST_ASSERT_TRUE(notify_stub_is_scheduled(NOTIFY_AUTOPHAGY_START));

    TEST_ASSERT_EQUAL_INT(T0 + 16 * H,  notify_stub_target_for(NOTIFY_FAST_COMPLETE));
    TEST_ASSERT_EQUAL_INT(T0 + 15 * H,  notify_stub_target_for(NOTIFY_FAST_1H_LEFT));
    TEST_ASSERT_EQUAL_INT(T0 +  8 * H,  notify_stub_target_for(NOTIFY_FAST_HALFWAY));
    TEST_ASSERT_EQUAL_INT(T0 + 12 * H,  notify_stub_target_for(NOTIFY_KETOSIS_START));
    // autophagy at 16h collides with goal, shifted 30 min earlier
    TEST_ASSERT_EQUAL_INT(T0 + 16 * H - COLLISION_SHIFT_S,
                          notify_stub_target_for(NOTIFY_AUTOPHAGY_START));
}

void test_milestones_13h_fast_ketosis_collision(void) {
    wakeup_setup();
    storage_set_milestones_on(true);
    notify_schedule_fast_milestones(T0, 13);

    // ketosis @12h collides with 1h-left @12h, shifts to 11h30
    TEST_ASSERT_EQUAL_INT(T0 + 12 * H - COLLISION_SHIFT_S,
                          notify_stub_target_for(NOTIFY_KETOSIS_START));
    TEST_ASSERT_EQUAL_INT(T0 + 12 * H,
                          notify_stub_target_for(NOTIFY_FAST_1H_LEFT));
    // autophagy @16h is past goal @13h, then shifted to 15h30 which is still past goal — dropped
    TEST_ASSERT_FALSE(notify_stub_is_scheduled(NOTIFY_AUTOPHAGY_START));
    TEST_ASSERT_TRUE(notify_stub_is_scheduled(NOTIFY_FAST_HALFWAY));
}

void test_milestones_10h_fast_drops_ketosis_and_autophagy(void) {
    wakeup_setup();
    storage_set_milestones_on(true);
    notify_schedule_fast_milestones(T0, 10);

    TEST_ASSERT_TRUE(notify_stub_is_scheduled(NOTIFY_FAST_COMPLETE));
    TEST_ASSERT_TRUE(notify_stub_is_scheduled(NOTIFY_FAST_1H_LEFT));
    TEST_ASSERT_TRUE(notify_stub_is_scheduled(NOTIFY_FAST_HALFWAY));
    TEST_ASSERT_FALSE(notify_stub_is_scheduled(NOTIFY_KETOSIS_START));
    TEST_ASSERT_FALSE(notify_stub_is_scheduled(NOTIFY_AUTOPHAGY_START));

    TEST_ASSERT_EQUAL_INT(T0 +  5 * H, notify_stub_target_for(NOTIFY_FAST_HALFWAY));
    TEST_ASSERT_EQUAL_INT(T0 +  9 * H, notify_stub_target_for(NOTIFY_FAST_1H_LEFT));
}

void test_milestones_disabled_skips_milestones_keeps_goal(void) {
    wakeup_setup();
    storage_set_milestones_on(false);
    notify_schedule_fast_milestones(T0, 16);

    TEST_ASSERT_TRUE(notify_stub_is_scheduled(NOTIFY_FAST_COMPLETE));
    TEST_ASSERT_FALSE(notify_stub_is_scheduled(NOTIFY_FAST_1H_LEFT));
    TEST_ASSERT_FALSE(notify_stub_is_scheduled(NOTIFY_FAST_HALFWAY));
    TEST_ASSERT_FALSE(notify_stub_is_scheduled(NOTIFY_KETOSIS_START));
    TEST_ASSERT_FALSE(notify_stub_is_scheduled(NOTIFY_AUTOPHAGY_START));
    TEST_ASSERT_EQUAL_INT(1, notify_stub_scheduled_count());
}

void test_milestones_eat_window_schedule_unaffected(void) {
    // Stopping a fast then scheduling eat window should not add milestone events
    wakeup_setup();
    storage_set_milestones_on(true);
    notify_schedule(NOTIFY_EAT_WINDOW_CLOSED, T0 + 8 * H);

    TEST_ASSERT_FALSE(notify_stub_is_scheduled(NOTIFY_FAST_HALFWAY));
    TEST_ASSERT_FALSE(notify_stub_is_scheduled(NOTIFY_KETOSIS_START));
    TEST_ASSERT_EQUAL_INT(1, notify_stub_scheduled_count());
}

void test_milestones_cancel_all_after_scheduling(void) {
    wakeup_setup();
    storage_set_milestones_on(true);
    notify_schedule_fast_milestones(T0, 16);
    int count_before = notify_stub_scheduled_count();
    TEST_ASSERT_TRUE(count_before > 1);

    notify_cancel_all_fasting();
    TEST_ASSERT_EQUAL_INT(0, notify_stub_scheduled_count());
}

void test_milestones_default_on_when_not_set(void) {
    persist_reset_all();
    TEST_ASSERT_TRUE(storage_get_milestones_on());
}
