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
void test_cancel_event_clears_storage(void);
void test_schedule_same_event_cancels_previous(void);
void test_schedule_different_events_independent(void);
void test_no_cancel_when_no_wakeup(void);
void test_cancel_all_fasting_cancels_every_event(void);
void test_milestones_16h_fast_schedules_all(void);
void test_milestones_13h_fast_ketosis_collision(void);
void test_milestones_10h_fast_drops_ketosis_and_autophagy(void);
void test_milestones_disabled_skips_milestones_keeps_goal(void);
void test_milestones_eat_window_schedule_unaffected(void);
void test_milestones_cancel_all_after_scheduling(void);
void test_milestones_default_on_when_not_set(void);

// test_glance.c
void test_glance_none_without_program(void);
void test_glance_none_when_ready(void);
void test_glance_active_fast_counts_to_goal(void);
void test_glance_eating_counts_to_window_end(void);
void test_glance_omad_counts_since_meal(void);

// test_actions.c
void test_state_ready_without_program(void);
void test_select_starts_fast_and_schedules_goal(void);
void test_select_stops_fast_opens_eat_window_alert(void);
void test_select_from_eating_restarts_fast(void);
void test_select_omad_second_meal_schedules_target(void);
void test_choose_program_clears_state_and_alerts(void);
void test_reset_all_clears_program_and_alerts(void);

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
    RUN_TEST(test_cancel_event_clears_storage);
    RUN_TEST(test_schedule_same_event_cancels_previous);
    RUN_TEST(test_schedule_different_events_independent);
    RUN_TEST(test_no_cancel_when_no_wakeup);
    RUN_TEST(test_cancel_all_fasting_cancels_every_event);
    RUN_TEST(test_milestones_16h_fast_schedules_all);
    RUN_TEST(test_milestones_13h_fast_ketosis_collision);
    RUN_TEST(test_milestones_10h_fast_drops_ketosis_and_autophagy);
    RUN_TEST(test_milestones_disabled_skips_milestones_keeps_goal);
    RUN_TEST(test_milestones_eat_window_schedule_unaffected);
    RUN_TEST(test_milestones_cancel_all_after_scheduling);
    RUN_TEST(test_milestones_default_on_when_not_set);


    RUN_TEST(test_glance_none_without_program);
    RUN_TEST(test_glance_none_when_ready);
    RUN_TEST(test_glance_active_fast_counts_to_goal);
    RUN_TEST(test_glance_eating_counts_to_window_end);
    RUN_TEST(test_glance_omad_counts_since_meal);

    RUN_TEST(test_state_ready_without_program);
    RUN_TEST(test_select_starts_fast_and_schedules_goal);
    RUN_TEST(test_select_stops_fast_opens_eat_window_alert);
    RUN_TEST(test_select_from_eating_restarts_fast);
    RUN_TEST(test_select_omad_second_meal_schedules_target);
    RUN_TEST(test_choose_program_clears_state_and_alerts);
    RUN_TEST(test_reset_all_clears_program_and_alerts);

    return UNITY_END();
}
