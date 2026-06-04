#include "unity.h"
#include "fasting.h"
#include "storage.h"
#include "platform/time.h"
#include "platform/persist.h"

extern int32_t g_now;
extern PersistSlot g_persist[PERSIST_MAX_SLOTS];

#define T0 1000000

// ── Timer math ────────────────────────────────────────────────────────────────

void test_elapsed_at_start(void) {
    TEST_ASSERT_EQUAL_INT(0, fast_elapsed(T0));
}

void test_elapsed_advances(void) {
    g_now = T0 + 3600;
    TEST_ASSERT_EQUAL_INT(3600, fast_elapsed(T0));
}

void test_remaining_at_start(void) {
    // 16h target: remaining = 16*3600 at T0
    TEST_ASSERT_EQUAL_INT(16 * 3600, fast_remaining(T0, 16));
}

void test_remaining_one_before_target(void) {
    g_now = T0 + 16 * 3600 - 1;
    TEST_ASSERT_EQUAL_INT(1, fast_remaining(T0, 16));
}

void test_remaining_at_target(void) {
    g_now = T0 + 16 * 3600;
    TEST_ASSERT_EQUAL_INT(0, fast_remaining(T0, 16));
}

void test_remaining_one_past_target(void) {
    g_now = T0 + 16 * 3600 + 1;
    TEST_ASSERT_EQUAL_INT(-1, fast_remaining(T0, 16));
}

void test_overtime_flag(void) {
    TEST_ASSERT_FALSE(fast_is_overtime(T0, 16));
    g_now = T0 + 16 * 3600;
    TEST_ASSERT_FALSE(fast_is_overtime(T0, 16)); // exactly at target is not overtime
    g_now = T0 + 16 * 3600 + 1;
    TEST_ASSERT_TRUE(fast_is_overtime(T0, 16));
}

// ── Start / stop ──────────────────────────────────────────────────────────────

void test_start_fast_stores_state(void) {
    start_fast(2); // 16:8
    TEST_ASSERT_EQUAL_INT(T0, storage_get_fast_started_at());
    TEST_ASSERT_EQUAL_INT(16, storage_get_fast_target_hours());
    TEST_ASSERT_EQUAL_INT(2,  storage_get_program_id());
}

void test_stop_fast_clears_active(void) {
    start_fast(2);
    g_now = T0 + 3600;
    stop_fast();
    TEST_ASSERT_EQUAL_INT(0, storage_get_fast_started_at());
    TEST_ASSERT_EQUAL_INT(0, storage_get_fast_target_hours());
}

void test_stop_fast_records_history(void) {
    start_fast(2);
    g_now = T0 + 3600;
    stop_fast();

    FastStats st = compute_fast_stats();
    TEST_ASSERT_EQUAL_UINT32(1,    st.total_fasts);
    TEST_ASSERT_EQUAL_UINT32(3600, st.avg_seconds);
    TEST_ASSERT_EQUAL_UINT32(3600, st.longest_seconds);
    TEST_ASSERT_EQUAL_UINT32(0,    st.total_overtime_seconds);
}

void test_stop_fast_records_overtime(void) {
    start_fast(2); // 16h target
    g_now = T0 + 16 * 3600 + 900; // 15 min overtime
    stop_fast();

    FastStats st = compute_fast_stats();
    TEST_ASSERT_EQUAL_UINT32(900, st.total_overtime_seconds);
}

void test_stop_fast_opens_eating_window(void) {
    start_fast(2); // 16:8 -> eat_hours = 8
    g_now = T0 + 3600;
    stop_fast();
    TEST_ASSERT_EQUAL_INT(g_now, storage_get_eat_started_at());
    TEST_ASSERT_EQUAL_INT(8,     storage_get_eat_target_hours());
}

void test_start_fast_closes_eating_window(void) {
    // Simulate an open eating window.
    storage_set_eat_started_at(T0);
    storage_set_eat_target_hours(8);

    g_now = T0 + 1000;
    start_fast(2); // should close eating window first
    TEST_ASSERT_EQUAL_INT(0, storage_get_eat_started_at());
}

void test_stop_fast_no_eat_window_for_omad(void) {
    start_fast(5); // OMAD — eat_hours=1, is_omad=true
    g_now = T0 + 3600;
    stop_fast();
    // OMAD path does not open an eating window via stop_fast
    // (OMAD uses log_meal instead). The eat window should stay 0.
    TEST_ASSERT_EQUAL_INT(0, storage_get_eat_started_at());
}

// ── Format ────────────────────────────────────────────────────────────────────

void test_format_hms_zero(void) {
    char buf[9];
    fast_format_hms(0, buf);
    TEST_ASSERT_EQUAL_STRING("00:00:00", buf);
}

void test_format_hms_negative(void) {
    char buf[9];
    fast_format_hms(-3661, buf); // abs = 3661 = 1h 1m 1s
    TEST_ASSERT_EQUAL_STRING("01:01:01", buf);
}

void test_format_hms_large(void) {
    char buf[9];
    fast_format_hms(23 * 3600, buf);
    TEST_ASSERT_EQUAL_STRING("23:00:00", buf);
}
