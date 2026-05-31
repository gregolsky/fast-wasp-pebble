#include "unity.h"
#include "fasting.h"
#include "storage.h"
#include "platform/time.h"
#include "platform/persist.h"

extern int32_t g_now;
extern PersistSlot g_persist[PERSIST_MAX_SLOTS];

#define T0 2000000

void test_first_meal_sets_last_meal_at(void) {
    g_now = T0;
    log_meal();
    TEST_ASSERT_EQUAL_INT(T0, storage_get_omad_last_meal_at());
}

void test_first_meal_no_history(void) {
    g_now = T0;
    log_meal();
    HistoryEntry buf[1];
    uint8_t n = storage_read_fast_history(buf, 1);
    TEST_ASSERT_EQUAL(0, n);
}

void test_second_meal_closes_previous_interval(void) {
    g_now = T0;
    log_meal();
    g_now = T0 + 23 * 3600;
    log_meal();

    HistoryEntry buf[1];
    uint8_t n = storage_read_fast_history(buf, 1);
    TEST_ASSERT_EQUAL(1, n);
    TEST_ASSERT_EQUAL_INT(T0,        buf[0].started_at);
    TEST_ASSERT_EQUAL_INT(23 * 3600, buf[0].duration_sec);
    TEST_ASSERT_EQUAL_INT(0,         buf[0].overtime_sec);
}

void test_second_meal_updates_last_meal_at(void) {
    g_now = T0;
    log_meal();
    g_now = T0 + 23 * 3600;
    log_meal();
    TEST_ASSERT_EQUAL_INT(T0 + 23 * 3600, storage_get_omad_last_meal_at());
}

void test_omad_overtime_recorded(void) {
    g_now = T0;
    log_meal();
    g_now = T0 + 24 * 3600;
    log_meal();

    HistoryEntry buf[1];
    storage_read_fast_history(buf, 1);
    TEST_ASSERT_EQUAL_INT(3600, buf[0].overtime_sec);
}

void test_elapsed_before_target(void) {
    g_now = T0;
    log_meal();
    g_now = T0 + 10 * 3600;
    int32_t el = fast_elapsed(storage_get_omad_last_meal_at());
    TEST_ASSERT_EQUAL_INT(10 * 3600, el);
    TEST_ASSERT_FALSE(fast_is_overtime(storage_get_omad_last_meal_at(), 23));
}

void test_elapsed_at_23h(void) {
    g_now = T0;
    log_meal();
    g_now = T0 + 23 * 3600;
    TEST_ASSERT_FALSE(fast_is_overtime(storage_get_omad_last_meal_at(), 23));
    g_now = T0 + 23 * 3600 + 1;
    TEST_ASSERT_TRUE(fast_is_overtime(storage_get_omad_last_meal_at(), 23));
}
