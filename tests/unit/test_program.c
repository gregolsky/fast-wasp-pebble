#include "unity.h"
#include "fasting.h"
#include "platform/time.h"
#include "platform/persist.h"

extern int32_t g_now;
extern PersistSlot g_persist[PERSIST_MAX_SLOTS];

void test_program_count(void) {
    TEST_ASSERT_EQUAL(6, NUM_PROGRAMS);
}

void test_program_labels(void) {
    TEST_ASSERT_EQUAL_STRING("12:12", PROGRAMS[0].label);
    TEST_ASSERT_EQUAL_STRING("14:10", PROGRAMS[1].label);
    TEST_ASSERT_EQUAL_STRING("16:8",  PROGRAMS[2].label);
    TEST_ASSERT_EQUAL_STRING("18:6",  PROGRAMS[3].label);
    TEST_ASSERT_EQUAL_STRING("20:4",  PROGRAMS[4].label);
    TEST_ASSERT_EQUAL_STRING("OMAD",  PROGRAMS[5].label);
}

void test_program_fast_hours(void) {
    TEST_ASSERT_EQUAL(12, PROGRAMS[0].fast_hours);
    TEST_ASSERT_EQUAL(14, PROGRAMS[1].fast_hours);
    TEST_ASSERT_EQUAL(16, PROGRAMS[2].fast_hours);
    TEST_ASSERT_EQUAL(18, PROGRAMS[3].fast_hours);
    TEST_ASSERT_EQUAL(20, PROGRAMS[4].fast_hours);
    TEST_ASSERT_EQUAL(23, PROGRAMS[5].fast_hours);
}

void test_program_eat_hours(void) {
    TEST_ASSERT_EQUAL(12, PROGRAMS[0].eat_hours);
    TEST_ASSERT_EQUAL(10, PROGRAMS[1].eat_hours);
    TEST_ASSERT_EQUAL( 8, PROGRAMS[2].eat_hours);
    TEST_ASSERT_EQUAL( 6, PROGRAMS[3].eat_hours);
    TEST_ASSERT_EQUAL( 4, PROGRAMS[4].eat_hours);
    TEST_ASSERT_EQUAL( 1, PROGRAMS[5].eat_hours);
}

void test_omad_flag(void) {
    TEST_ASSERT_FALSE(PROGRAMS[0].is_omad);
    TEST_ASSERT_FALSE(PROGRAMS[4].is_omad);
    TEST_ASSERT_TRUE(PROGRAMS[5].is_omad);
}

void test_program_by_index_clamps(void) {
    TEST_ASSERT_EQUAL_PTR(&PROGRAMS[0], program_by_index(0));
    TEST_ASSERT_EQUAL_PTR(&PROGRAMS[5], program_by_index(5));
    TEST_ASSERT_EQUAL_PTR(&PROGRAMS[0], program_by_index(99)); // out-of-range -> 0
}
