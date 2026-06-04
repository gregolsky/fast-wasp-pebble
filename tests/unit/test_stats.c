#include "unity.h"
#include "fasting.h"
#include "storage.h"
#include "platform/time.h"
#include "platform/persist.h"

extern int32_t g_now;
extern PersistSlot g_persist[PERSIST_MAX_SLOTS];

void test_stats_empty(void) {
    FastStats st = compute_fast_stats();
    TEST_ASSERT_EQUAL_UINT32(0, st.total_fasts);
    TEST_ASSERT_EQUAL_UINT32(0, st.avg_seconds);
    TEST_ASSERT_EQUAL_UINT32(0, st.longest_seconds);
    TEST_ASSERT_EQUAL_UINT32(0, st.total_overtime_seconds);
}

void test_stats_one_record(void) {
    storage_record_fast(3600, 0);
    FastStats st = compute_fast_stats();
    TEST_ASSERT_EQUAL_UINT32(1,    st.total_fasts);
    TEST_ASSERT_EQUAL_UINT32(3600, st.avg_seconds);
    TEST_ASSERT_EQUAL_UINT32(3600, st.longest_seconds);
    TEST_ASSERT_EQUAL_UINT32(0,    st.total_overtime_seconds);
}

void test_stats_accumulates(void) {
    storage_record_fast(3600, 0);
    storage_record_fast(7200, 300);
    FastStats st = compute_fast_stats();
    TEST_ASSERT_EQUAL_UINT32(2,    st.total_fasts);
    TEST_ASSERT_EQUAL_UINT32(5400, st.avg_seconds);   // (3600+7200)/2
    TEST_ASSERT_EQUAL_UINT32(300,  st.total_overtime_seconds);
}

void test_stats_longest_tracked(void) {
    storage_record_fast(3600,  0);
    storage_record_fast(7200,  0);
    storage_record_fast(1800,  0);
    FastStats st = compute_fast_stats();
    TEST_ASSERT_EQUAL_UINT32(7200, st.longest_seconds);
}

void test_stats_reset_clears(void) {
    storage_record_fast(3600, 60);
    storage_reset_all();
    FastStats st = compute_fast_stats();
    TEST_ASSERT_EQUAL_UINT32(0, st.total_fasts);
    TEST_ASSERT_EQUAL_UINT32(0, st.avg_seconds);
    TEST_ASSERT_EQUAL_UINT32(0, st.longest_seconds);
    TEST_ASSERT_EQUAL_UINT32(0, st.total_overtime_seconds);
}
