#include "unity.h"
#include "glance.h"
#include "fasting.h"
#include "storage.h"
#include "platform/time.h"

#include <string.h>

extern int32_t g_now;

void test_glance_none_without_program(void) {
    char buf[150];
    TEST_ASSERT_FALSE(glance_subtitle(buf, sizeof(buf)));
}

void test_glance_none_when_ready(void) {
    char buf[150];
    storage_set_program_id(2);
    TEST_ASSERT_FALSE(glance_subtitle(buf, sizeof(buf)));
}

void test_glance_active_fast_counts_to_goal(void) {
    char buf[150];
    start_fast(2); // 16:8
    int32_t end = storage_get_fast_started_at() + 16 * 3600;
    char want[150];
    snprintf(want, sizeof(want),
             "{time_until(%ld)|format(>0S:'Fast ends in %%aR','Fast done! Open to eat')}",
             (long)end);
    TEST_ASSERT_TRUE(glance_subtitle(buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_STRING(want, buf);
}

void test_glance_eating_counts_to_window_end(void) {
    char buf[150];
    start_fast(2);
    stop_fast();
    TEST_ASSERT_TRUE(glance_subtitle(buf, sizeof(buf)));
    TEST_ASSERT_NOT_NULL(strstr(buf, "Eat window"));
}

void test_glance_omad_counts_since_meal(void) {
    char buf[150];
    storage_set_program_id(5);
    TEST_ASSERT_FALSE(glance_subtitle(buf, sizeof(buf)));
    log_meal();
    TEST_ASSERT_TRUE(glance_subtitle(buf, sizeof(buf)));
    TEST_ASSERT_NOT_NULL(strstr(buf, "time_since"));
}
