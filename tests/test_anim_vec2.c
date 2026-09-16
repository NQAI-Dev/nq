#include "nq/anim_vec2.h"
#include "test_main.c"

static void test_anim_vec2_init_starts_active(void) {
    NqAnimVec2 a;
    nq_anim_vec2_init(&a, nq_vec2f(0.0f, 0.0f), nq_vec2f(100.0f, 200.0f),
                       1.0f, NQ_EASE_LINEAR);
    NQ_ASSERT(nq_anim_vec2_done(&a) == 0);
    NqVec2f v = nq_anim_vec2_value(&a);
    /* At t=0 value is `from` (forward direction). */
    NQ_ASSERT(v.x == 0.0f);
    NQ_ASSERT(v.y == 0.0f);
}

static void test_anim_vec2_linear_progresses_to_target(void) {
    NqAnimVec2 a;
    nq_anim_vec2_init(&a, nq_vec2f(0.0f, 0.0f), nq_vec2f(100.0f, 200.0f),
                       1.0f, NQ_EASE_LINEAR);
    NQ_ASSERT_EQ(nq_anim_vec2_update(&a, 1.0f), 1);
    NQ_ASSERT(nq_anim_vec2_done(&a));
    NqVec2f v = nq_anim_vec2_value(&a);
    NQ_ASSERT(v.x == 100.0f);
    NQ_ASSERT(v.y == 200.0f);
}

static void test_anim_vec2_linear_midpoint(void) {
    /* 0,0 → 100,200 at t=0.5 → 50, 100 */
    NqAnimVec2 a;
    nq_anim_vec2_init(&a, nq_vec2f(0.0f, 0.0f), nq_vec2f(100.0f, 200.0f),
                       1.0f, NQ_EASE_LINEAR);
    NQ_ASSERT_EQ(nq_anim_vec2_update(&a, 0.5f), 0);
    NqVec2f v = nq_anim_vec2_value(&a);
    NQ_ASSERT(v.x == 50.0f);
    NQ_ASSERT(v.y == 100.0f);
}

static void test_anim_vec2_saturates_at_to(void) {
    /* Big dt → clamps at `to`, no overshoot. */
    NqAnimVec2 a;
    nq_anim_vec2_init(&a, nq_vec2f(10.0f, 20.0f), nq_vec2f(100.0f, 200.0f),
                       1.0f, NQ_EASE_LINEAR);
    nq_anim_vec2_update(&a, 100.0f);
    NqVec2f v = nq_anim_vec2_value(&a);
    NQ_ASSERT(v.x == 100.0f);
    NQ_ASSERT(v.y == 200.0f);
}

static void test_anim_vec2_reverse_direction(void) {
    /* from = (100, 200), to = (0, 0) → reverse flag set → at t=0 value is `to`. */
    NqAnimVec2 a;
    nq_anim_vec2_init(&a, nq_vec2f(100.0f, 200.0f), nq_vec2f(0.0f, 0.0f),
                       1.0f, NQ_EASE_LINEAR);
    NqVec2f v0 = nq_anim_vec2_value(&a);
    NQ_ASSERT(v0.x == 0.0f);
    NQ_ASSERT(v0.y == 0.0f);
    /* Run to completion, value reaches `from`. */
    nq_anim_vec2_update(&a, 1.0f);
    NqVec2f v1 = nq_anim_vec2_value(&a);
    NQ_ASSERT(v1.x == 100.0f);
    NQ_ASSERT(v1.y == 200.0f);
}

static void test_anim_vec2_zero_duration_clamps(void) {
    NqAnimVec2 a;
    nq_anim_vec2_init(&a, nq_vec2f(0.0f, 0.0f), nq_vec2f(100.0f, 200.0f),
                       0.0f, NQ_EASE_LINEAR);
    /* First positive dt completes. */
    NQ_ASSERT_EQ(nq_anim_vec2_update(&a, 0.016f), 1);
    NQ_ASSERT(nq_anim_vec2_done(&a));
}

static void test_anim_vec2_negative_dt_clamped(void) {
    NqAnimVec2 a;
    nq_anim_vec2_init(&a, nq_vec2f(0.0f, 0.0f), nq_vec2f(100.0f, 200.0f),
                       1.0f, NQ_EASE_LINEAR);
    nq_anim_vec2_update(&a, -0.5f);
    NqVec2f v = nq_anim_vec2_value(&a);
    NQ_ASSERT(v.x == 0.0f);   /* still at `from`, elapsed clamped to 0 */
}

static void test_anim_vec2_restart_resets(void) {
    NqAnimVec2 a;
    nq_anim_vec2_init(&a, nq_vec2f(0.0f, 0.0f), nq_vec2f(100.0f, 200.0f),
                       1.0f, NQ_EASE_LINEAR);
    nq_anim_vec2_update(&a, 1.0f);
    NQ_ASSERT(nq_anim_vec2_done(&a));
    nq_anim_vec2_restart(&a);
    NQ_ASSERT(nq_anim_vec2_done(&a) == 0);
    NqVec2f v = nq_anim_vec2_value(&a);
    NQ_ASSERT(v.x == 0.0f);
}

static void test_anim_vec2_null_safe(void) {
    nq_anim_vec2_init(NULL,
        nq_vec2f(0.0f, 0.0f), nq_vec2f(1.0f, 1.0f), 1.0f, NQ_EASE_LINEAR);
    NQ_ASSERT_EQ(nq_anim_vec2_update(NULL, 0.016f), 0);
    NQ_ASSERT(nq_anim_vec2_done(NULL));
    NqVec2f v = nq_anim_vec2_value(NULL);
    NQ_ASSERT(v.x == 0.0f);
    NQ_ASSERT(v.y == 0.0f);
    nq_anim_vec2_restart(NULL);
}

NQ_TEST_REGISTER("anim_vec2_init_starts_active", test_anim_vec2_init_starts_active);
NQ_TEST_REGISTER("anim_vec2_linear_progresses", test_anim_vec2_linear_progresses_to_target);
NQ_TEST_REGISTER("anim_vec2_linear_midpoint",    test_anim_vec2_linear_midpoint);
NQ_TEST_REGISTER("anim_vec2_saturates",          test_anim_vec2_saturates_at_to);
NQ_TEST_REGISTER("anim_vec2_reverse_direction", test_anim_vec2_reverse_direction);
NQ_TEST_REGISTER("anim_vec2_zero_duration",     test_anim_vec2_zero_duration_clamps);
NQ_TEST_REGISTER("anim_vec2_negative_dt",        test_anim_vec2_negative_dt_clamped);
NQ_TEST_REGISTER("anim_vec2_restart_resets",    test_anim_vec2_restart_resets);
NQ_TEST_REGISTER("anim_vec2_null_safe",         test_anim_vec2_null_safe);
