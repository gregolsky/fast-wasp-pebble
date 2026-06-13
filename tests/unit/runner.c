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
void test_eating_window_restart_fast(void);
void test_eating_window_end(void);
void test_start_fast_clamps_out_of_range(void);
void test_eating_window_remaining_at_start(void);
void test_eating_window_remaining_one_before_target(void);
void test_eating_window_remaining_at_target(void);
void test_eating_window_overtime_one_second_past(void);
void test_eating_window_overtime_one_minute_past(void);
void test_eating_window_target_hours_zero_is_safe(void);
void test_format_hm_zero(void);
void test_format_hm_negative(void);
void test_format_hm_large(void);
void test_storage_program_id_default(void);
void test_storage_wakeup_id_default(void);
void test_storage_vibration_default(void);
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

// test_stats.c
void test_stats_empty(void);
void test_stats_one_record(void);
void test_stats_accumulates(void);
void test_stats_longest_tracked(void);
void test_stats_reset_clears(void);

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
    RUN_TEST(test_eating_window_restart_fast);
    RUN_TEST(test_eating_window_end);
    RUN_TEST(test_start_fast_clamps_out_of_range);
    RUN_TEST(test_eating_window_remaining_at_start);
    RUN_TEST(test_eating_window_remaining_one_before_target);
    RUN_TEST(test_eating_window_remaining_at_target);
    RUN_TEST(test_eating_window_overtime_one_second_past);
    RUN_TEST(test_eating_window_overtime_one_minute_past);
    RUN_TEST(test_eating_window_target_hours_zero_is_safe);
    RUN_TEST(test_format_hm_zero);
    RUN_TEST(test_format_hm_negative);
    RUN_TEST(test_format_hm_large);
    RUN_TEST(test_storage_program_id_default);
    RUN_TEST(test_storage_wakeup_id_default);
    RUN_TEST(test_storage_vibration_default);
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

    RUN_TEST(test_stats_empty);
    RUN_TEST(test_stats_one_record);
    RUN_TEST(test_stats_accumulates);
    RUN_TEST(test_stats_longest_tracked);
    RUN_TEST(test_stats_reset_clears);

    RUN_TEST(test_wakeup_target_for_16h_fast);
    RUN_TEST(test_cancel_on_state_change);
    RUN_TEST(test_schedule_cancels_previous);
    RUN_TEST(test_no_cancel_when_no_wakeup);

    return UNITY_END();
}
