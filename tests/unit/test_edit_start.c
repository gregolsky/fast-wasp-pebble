#include "unity.h"
#include "fasting.h"
#include "storage.h"
#include "platform/time.h"
#include "platform/persist.h"

extern int32_t g_now;
extern PersistSlot g_persist[PERSIST_MAX_SLOTS];

#define T0 3000000

static void edit_start_setup(void) {
    persist_reset_all();
    g_now = T0;
    start_fast(2); // 16:8; fast started_at = T0
    g_now = T0 + 2 * 3600; // now = T0+2h
}

void test_offset_zero_ok(void) {
    edit_start_setup();
    EditStartResult r = apply_fast_start_offset(0);
    TEST_ASSERT_EQUAL(EDIT_START_OK, r);
    TEST_ASSERT_EQUAL_INT(T0, storage_get_fast_started_at());
}

void test_offset_minus_15_ok(void) {
    edit_start_setup();
    EditStartResult r = apply_fast_start_offset(-15);
    TEST_ASSERT_EQUAL(EDIT_START_OK, r);
    TEST_ASSERT_EQUAL_INT(T0 - 15 * 60, storage_get_fast_started_at());
}

void test_offset_plus_15_ok(void) {
    edit_start_setup();
    // T0+15min is still in the past (now = T0+2h).
    EditStartResult r = apply_fast_start_offset(15);
    TEST_ASSERT_EQUAL(EDIT_START_OK, r);
    TEST_ASSERT_EQUAL_INT(T0 + 15 * 60, storage_get_fast_started_at());
}

void test_future_commit_refused(void) {
    edit_start_setup();
    // +3h -> new start at T0+3h; now = T0+2h -> future.
    EditStartResult r = apply_fast_start_offset(3 * 60);
    TEST_ASSERT_EQUAL(EDIT_START_FUTURE, r);
    TEST_ASSERT_EQUAL_INT(T0, storage_get_fast_started_at());
}

void test_too_old_refused(void) {
    edit_start_setup();
    int32_t old_offset = -(15 * 24 * 60);
    EditStartResult r = apply_fast_start_offset(old_offset);
    TEST_ASSERT_EQUAL(EDIT_START_TOO_OLD, r);
    TEST_ASSERT_EQUAL_INT(T0, storage_get_fast_started_at());
}

void test_exactly_14_days_ago_ok(void) {
    edit_start_setup();
    int32_t now    = g_now;              // T0 + 2h
    int32_t stored = storage_get_fast_started_at(); // T0
    // Target new_start = now - 14*24*3600 (exactly at boundary)
    int32_t target     = now - 14 * 24 * 3600;
    int32_t offset_min = (target - stored) / 60;
    EditStartResult r  = apply_fast_start_offset(offset_min);
    TEST_ASSERT_EQUAL(EDIT_START_OK, r);
}
