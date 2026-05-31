#include "unity.h"
#include "storage.h"
#include "platform/time.h"
#include "platform/persist.h"

extern int32_t g_now;
extern PersistSlot g_persist[PERSIST_MAX_SLOTS];

static HistoryEntry make_entry(int32_t at, int32_t dur, int32_t ot) {
    HistoryEntry e = { .started_at = at, .duration_sec = dur, .overtime_sec = ot };
    return e;
}

void test_empty_ring(void) {
    persist_reset_all();
    HistoryEntry buf[1];
    TEST_ASSERT_EQUAL(0, storage_read_fast_history(buf, 1));
}

void test_push_one(void) {
    persist_reset_all();
    HistoryEntry e = make_entry(100, 3600, 0);
    storage_push_fast_history(&e);

    HistoryEntry buf[1];
    uint8_t n = storage_read_fast_history(buf, 1);
    TEST_ASSERT_EQUAL(1, n);
    TEST_ASSERT_EQUAL_INT(100,  buf[0].started_at);
    TEST_ASSERT_EQUAL_INT(3600, buf[0].duration_sec);
}

void test_push_three_chronological_order(void) {
    persist_reset_all();
    for (int i = 0; i < 3; i++) {
        HistoryEntry e = make_entry(i * 1000, i * 3600, 0);
        storage_push_fast_history(&e);
    }
    HistoryEntry buf[3];
    uint8_t n = storage_read_fast_history(buf, 3);
    TEST_ASSERT_EQUAL(3, n);
    for (int i = 0; i < 3; i++) {
        TEST_ASSERT_EQUAL_INT(i * 1000, buf[i].started_at);
    }
}

void test_ring_wraps_at_capacity(void) {
    persist_reset_all();
    // Fill past capacity; oldest entry (i=0) should be evicted.
    for (int i = 0; i <= (int)FAST_HISTORY_CAPACITY; i++) {
        HistoryEntry e = make_entry(i, i * 60, 0);
        storage_push_fast_history(&e);
    }
    HistoryEntry buf[FAST_HISTORY_CAPACITY];
    uint8_t n = storage_read_fast_history(buf, FAST_HISTORY_CAPACITY);
    TEST_ASSERT_EQUAL(FAST_HISTORY_CAPACITY, n);
    TEST_ASSERT_EQUAL_INT(1, buf[0].started_at); // i=0 evicted; i=1 is oldest
}

void test_budget_under_4kb(void) {
    persist_reset_all();
    for (int i = 0; i < (int)FAST_HISTORY_CAPACITY; i++) {
        HistoryEntry e = make_entry(i, 3600, 0);
        storage_push_fast_history(&e);
    }
    int sz = storage_history_bytes();
    TEST_ASSERT_TRUE(sz > 0);
    TEST_ASSERT_TRUE(sz < 4096);
}
