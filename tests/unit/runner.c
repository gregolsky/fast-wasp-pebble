// Calls all test suites.
#include "unity.h"
#include "platform/time.h"
#include "platform/persist.h"
#include <stdio.h>

extern int32_t g_now;
extern PersistSlot g_persist[PERSIST_MAX_SLOTS];

// Global setUp/tearDown: reset shared state before every test.
void setUp(void)    { persist_reset_all(); g_now = 1000000; }
void tearDown(void) {}

// test_program.c
void test_program_count(void);
void test_program_labels(void);
void test_program_fast_hours(void);
void test_program_eat_hours(void);
void test_omad_flag(void);
void test_program_by_index_clamps(void);

// test_fasting.c
void test_elapsed_at_start(void);
void test_elapsed_advances(void);
void test_remaining_at_start(void);
void test_remaining_one_before_target(void);
void test_remaining_at_target(void);
void test_remaining_one_past_target(void);
void test_overtime_flag(void);
void test_start_fast_stores_state(void);
void test_stop_fast_clears_active(void);
void test_stop_fast_records_history(void);
void test_stop_fast_records_overtime(void);
void test_stop_fast_opens_eating_window(void);
void test_start_fast_closes_eating_window(void);
void test_stop_fast_no_eat_window_for_omad(void);
void test_format_hms_zero(void);
void test_format_hms_negative(void);
void test_format_hms_large(void);

// test_omad.c
void test_first_meal_sets_last_meal_at(void);
void test_first_meal_no_history(void);
void test_second_meal_closes_previous_interval(void);
void test_second_meal_updates_last_meal_at(void);
void test_omad_overtime_recorded(void);
void test_elapsed_before_target(void);
void test_elapsed_at_23h(void);

// test_edit_start.c
void test_offset_zero_ok(void);
void test_offset_minus_15_ok(void);
void test_offset_plus_15_ok(void);
void test_future_commit_refused(void);
void test_too_old_refused(void);
void test_exactly_14_days_ago_ok(void);

// test_ring.c
void test_empty_ring(void);
void test_push_one(void);
void test_push_three_chronological_order(void);
void test_ring_wraps_at_capacity(void);
void test_budget_under_4kb(void);

// test_wakeup.c
void test_wakeup_target_for_16h_fast(void);
void test_cancel_on_state_change(void);
void test_schedule_cancels_previous(void);
void test_no_cancel_when_no_wakeup(void);

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_program_count);
    RUN_TEST(test_program_labels);
    RUN_TEST(test_program_fast_hours);
    RUN_TEST(test_program_eat_hours);
    RUN_TEST(test_omad_flag);
    RUN_TEST(test_program_by_index_clamps);

    RUN_TEST(test_elapsed_at_start);
    RUN_TEST(test_elapsed_advances);
    RUN_TEST(test_remaining_at_start);
    RUN_TEST(test_remaining_one_before_target);
    RUN_TEST(test_remaining_at_target);
    RUN_TEST(test_remaining_one_past_target);
    RUN_TEST(test_overtime_flag);
    RUN_TEST(test_start_fast_stores_state);
    RUN_TEST(test_stop_fast_clears_active);
    RUN_TEST(test_stop_fast_records_history);
    RUN_TEST(test_stop_fast_records_overtime);
    RUN_TEST(test_stop_fast_opens_eating_window);
    RUN_TEST(test_start_fast_closes_eating_window);
    RUN_TEST(test_stop_fast_no_eat_window_for_omad);
    RUN_TEST(test_format_hms_zero);
    RUN_TEST(test_format_hms_negative);
    RUN_TEST(test_format_hms_large);

    RUN_TEST(test_first_meal_sets_last_meal_at);
    RUN_TEST(test_first_meal_no_history);
    RUN_TEST(test_second_meal_closes_previous_interval);
    RUN_TEST(test_second_meal_updates_last_meal_at);
    RUN_TEST(test_omad_overtime_recorded);
    RUN_TEST(test_elapsed_before_target);
    RUN_TEST(test_elapsed_at_23h);

    RUN_TEST(test_offset_zero_ok);
    RUN_TEST(test_offset_minus_15_ok);
    RUN_TEST(test_offset_plus_15_ok);
    RUN_TEST(test_future_commit_refused);
    RUN_TEST(test_too_old_refused);
    RUN_TEST(test_exactly_14_days_ago_ok);

    RUN_TEST(test_empty_ring);
    RUN_TEST(test_push_one);
    RUN_TEST(test_push_three_chronological_order);
    RUN_TEST(test_ring_wraps_at_capacity);
    RUN_TEST(test_budget_under_4kb);

    RUN_TEST(test_wakeup_target_for_16h_fast);
    RUN_TEST(test_cancel_on_state_change);
    RUN_TEST(test_schedule_cancels_previous);
    RUN_TEST(test_no_cancel_when_no_wakeup);

    return UNITY_END();
}
