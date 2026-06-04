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
    FastStats st = compute_fast_stats();
    TEST_ASSERT_EQUAL_UINT32(0, st.total_fasts);
}

void test_second_meal_closes_previous_interval(void) {
    g_now = T0;
    log_meal();
    g_now = T0 + 23 * 3600;
    log_meal();

    FastStats st = compute_fast_stats();
    TEST_ASSERT_EQUAL_UINT32(1,          st.total_fasts);
    TEST_ASSERT_EQUAL_UINT32(23 * 3600,  st.avg_seconds);
    TEST_ASSERT_EQUAL_UINT32(0,          st.total_overtime_seconds);
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

    FastStats st = compute_fast_stats();
    TEST_ASSERT_EQUAL_UINT32(3600, st.total_overtime_seconds);
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
