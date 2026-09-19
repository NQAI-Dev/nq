#include <nq/action_delay.h>
#include "test_main.c"
#define NQ_FE(a, b) NQ_ASSERT((a) - (b) < 1e-5f && (b) - (a) < 1e-5f)

static void test_delay_create_destroy(void) {
    NqActionDelay *d = nq_action_delay_create(1.0f);
    NQ_ASSERT(d != NULL);
    NQ_ASSERT(nq_action_delay_action(d) != NULL);
    NQ_FE(nq_action_delay_duration(d), 1.0f);
    NQ_FE(nq_action_delay_elapsed(d), 0.0f);
    nq_action_delay_destroy(d);
}

static void test_delay_runs_until_duration(void) {
    /* 0.5s duration. After 4 ticks of 0.1s each (0.4s), still RUNNING. */
    NqActionDelay *d = nq_action_delay_create(0.5f);
    for (int i = 0; i < 4; i++) {
        NQ_ASSERT_EQ(nq_action_delay_update(d, 0.1f), NQ_ACTION_RUNNING);
    }
    /* 5th tick pushes us to 0.5s — FINISHED */
    NQ_ASSERT_EQ(nq_action_delay_update(d, 0.1f), NQ_ACTION_FINISHED);
    NQ_FE(nq_action_delay_elapsed(d), 0.5f);
    nq_action_delay_destroy(d);
}

static void test_delay_saturates_at_duration(void) {
    /* Big dt — value clamps to duration, no overshoot */
    NqActionDelay *d = nq_action_delay_create(1.0f);
    nq_action_delay_update(d, 100.0f);
    NQ_FE(nq_action_delay_elapsed(d), 1.0f);
    nq_action_delay_destroy(d);
}

static void test_delay_zero_duration_finishes_immediately(void) {
    NqActionDelay *d = nq_action_delay_create(0.0f);
    /* Zero-duration: first tick must complete immediately. */
    NQ_ASSERT_EQ(nq_action_delay_update(d, 0.016f), NQ_ACTION_FINISHED);
    nq_action_delay_destroy(d);
}

static void test_delay_negative_duration_treated_as_zero(void) {
    /* Negative duration is clamped to 0 — must finish immediately. */
    NqActionDelay *d = nq_action_delay_create(-5.0f);
    NQ_FE(nq_action_delay_duration(d), 0.0f);
    NQ_ASSERT_EQ(nq_action_delay_update(d, 0.016f), NQ_ACTION_FINISHED);
    nq_action_delay_destroy(d);
}

static void test_delay_null_safe(void) {
    nq_action_delay_destroy(NULL);
    NQ_ASSERT_EQ(nq_action_delay_update(NULL, 0.016f), NQ_ACTION_FINISHED);
    NQ_ASSERT(nq_action_delay_action(NULL) == NULL);
    NQ_FE(nq_action_delay_elapsed(NULL), 0.0f);
    NQ_FE(nq_action_delay_duration(NULL), 0.0f);
}

NQ_TEST_REGISTER("delay_create_destroy",          test_delay_create_destroy)
NQ_TEST_REGISTER("delay_runs_until_duration",    test_delay_runs_until_duration)
NQ_TEST_REGISTER("delay_saturates_at_duration",  test_delay_saturates_at_duration)
NQ_TEST_REGISTER("delay_zero_finishes_immediate", test_delay_zero_duration_finishes_immediately)
NQ_TEST_REGISTER("delay_negative_clamped_to_zero", test_delay_negative_duration_treated_as_zero)
NQ_TEST_REGISTER("delay_null_safe",              test_delay_null_safe)
