#include "unity.h"
#include "fasting.h"
#include "storage.h"
#include "notify.h"
#include "platform/time.h"

extern int32_t g_now;

#define H 3600

static void actions_setup(uint8_t prog) {
    notify_stub_reset();
    if (prog != NO_PROGRAM) storage_set_program_id(prog);
}

void test_state_ready_without_program(void) {
    actions_setup(NO_PROGRAM);
    TEST_ASSERT_EQUAL(FAST_STATE_READY, fasting_state());
    TEST_ASSERT_FALSE(fasting_select());
    TEST_ASSERT_EQUAL_INT(0, notify_stub_scheduled_count());
}

void test_select_starts_fast_and_schedules_goal(void) {
    actions_setup(2); // 16:8
    TEST_ASSERT_TRUE(fasting_select());
    TEST_ASSERT_EQUAL(FAST_STATE_ACTIVE, fasting_state());
    TEST_ASSERT_EQUAL_INT(g_now + 16 * H, notify_stub_target_for(NOTIFY_FAST_COMPLETE));
}

void test_select_stops_fast_opens_eat_window_alert(void) {
    actions_setup(2);
    fasting_select();
    g_now += 16 * H;
    fasting_select();
    TEST_ASSERT_EQUAL(FAST_STATE_EATING, fasting_state());
    TEST_ASSERT_FALSE(notify_stub_is_scheduled(NOTIFY_FAST_COMPLETE));
    TEST_ASSERT_EQUAL_INT(g_now + 8 * H, notify_stub_target_for(NOTIFY_EAT_WINDOW_CLOSED));
}

void test_select_from_eating_restarts_fast(void) {
    actions_setup(2);
    fasting_select();
    fasting_select();
    g_now += H;
    fasting_select();
    TEST_ASSERT_EQUAL(FAST_STATE_ACTIVE, fasting_state());
    TEST_ASSERT_FALSE(notify_stub_is_scheduled(NOTIFY_EAT_WINDOW_CLOSED));
    TEST_ASSERT_EQUAL_INT(g_now + 16 * H, notify_stub_target_for(NOTIFY_FAST_COMPLETE));
}

void test_select_omad_first_meal_schedules_target(void) {
    actions_setup(5);
    fasting_select(); // first meal, from READY
    TEST_ASSERT_EQUAL(FAST_STATE_OMAD, fasting_state());
    TEST_ASSERT_EQUAL_INT(g_now + OMAD_HOURS * H, notify_stub_target_for(NOTIFY_OMAD_TARGET));
}

void test_select_omad_second_meal_schedules_target(void) {
    actions_setup(5);
    fasting_select(); // first meal
    TEST_ASSERT_EQUAL(FAST_STATE_OMAD, fasting_state());
    g_now += 2 * H;
    fasting_select(); // next meal
    TEST_ASSERT_EQUAL_INT(g_now + OMAD_HOURS * H, notify_stub_target_for(NOTIFY_OMAD_TARGET));
}

void test_choose_program_clears_state_and_alerts(void) {
    actions_setup(2);
    fasting_select();
    fasting_choose_program(0);
    TEST_ASSERT_EQUAL(FAST_STATE_READY, fasting_state());
    TEST_ASSERT_EQUAL_INT(0, storage_get_program_id());
    TEST_ASSERT_EQUAL_INT(0, notify_stub_scheduled_count());
}

void test_reset_all_clears_program_and_alerts(void) {
    actions_setup(2);
    fasting_select();
    fasting_reset_all();
    TEST_ASSERT_EQUAL_INT(NO_PROGRAM, storage_get_program_id());
    TEST_ASSERT_EQUAL_INT(0, notify_stub_scheduled_count());
}
