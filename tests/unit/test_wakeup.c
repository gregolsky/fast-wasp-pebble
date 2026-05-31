#include "unity.h"
#include "fasting.h"
#include "storage.h"
#include "notify.h"
#include "platform/time.h"
#include "platform/persist.h"

extern int32_t notify_stub_last_target(void);
extern int32_t notify_stub_last_event(void);
extern int     notify_stub_cancel_count(void);
extern void    notify_stub_reset(void);

extern int32_t g_now;
extern PersistSlot g_persist[PERSIST_MAX_SLOTS];

#define T0 5000000

static void wakeup_setup(void) {
    persist_reset_all();
    g_now = T0;
    notify_stub_reset();
}

void test_wakeup_target_for_16h_fast(void) {
    wakeup_setup();
    start_fast(2); // 16:8
    int32_t started  = storage_get_fast_started_at();
    uint8_t hours    = storage_get_fast_target_hours();
    int32_t expected = started + (int32_t)hours * 3600;

    notify_schedule(NOTIFY_FAST_COMPLETE, expected);
    TEST_ASSERT_EQUAL_INT(expected, notify_stub_last_target());
    TEST_ASSERT_EQUAL_INT(NOTIFY_FAST_COMPLETE, notify_stub_last_event());
}

void test_cancel_on_state_change(void) {
    wakeup_setup();
    start_fast(2);
    notify_schedule(NOTIFY_FAST_COMPLETE, T0 + 16 * 3600);
    TEST_ASSERT_EQUAL_INT(42, storage_get_wakeup_id()); // 42 = sentinel set by stub

    notify_cancel();
    TEST_ASSERT_EQUAL_INT(-1, storage_get_wakeup_id());
    TEST_ASSERT_EQUAL_INT(1,  notify_stub_cancel_count());
}

void test_schedule_cancels_previous(void) {
    wakeup_setup();
    notify_schedule(NOTIFY_FAST_COMPLETE, T0 + 16 * 3600);
    notify_schedule(NOTIFY_EAT_WINDOW_CLOSED, T0 + 24 * 3600);

    TEST_ASSERT_EQUAL_INT(1, notify_stub_cancel_count());
    TEST_ASSERT_EQUAL_INT(NOTIFY_EAT_WINDOW_CLOSED, notify_stub_last_event());
}

void test_no_cancel_when_no_wakeup(void) {
    wakeup_setup();
    notify_cancel();
    TEST_ASSERT_EQUAL_INT(0, notify_stub_cancel_count());
}
