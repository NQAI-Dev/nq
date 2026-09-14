#include <nq/animation.h>
#include "test_main.c"

#define NQ_EPS 1e-4f

static void test_init_starts_active(void) {
    NqAnimFloat a;
    nq_anim_float_init(&a, 0.0f, 10.0f, 1.0f, NQ_EASE_LINEAR);
    NQ_ASSERT_EQ(nq_anim_float_done(&a), 0);
    NQ_FE(nq_anim_float_value(&a), 0.0f);  /* t=0 -> from */
}

static void test_linear_progresses_to_target(void) {
    NqAnimFloat a;
    nq_anim_float_init(&a, 0.0f, 100.0f, 1.0f, NQ_EASE_LINEAR);
    /* Advance in 100ms steps for ~1s */
    for (int i = 1; i <= 10; i++) {
        (void)nq_anim_float_update(&a, 0.1f);
    }
    /* After 1s of linear 0->100, value should be ~100 (saturated at to) */
    NQ_ASSERT_EQ(nq_anim_float_done(&a), 1);
    NQ_FE(nq_anim_float_value(&a), 100.0f);
}

static void test_linear_at_half_duration(void) {
    NqAnimFloat a;
    nq_anim_float_init(&a, 0.0f, 100.0f, 1.0f, NQ_EASE_LINEAR);
    (void)nq_anim_float_update(&a, 0.5f);
    /* Linear 0->100 at 0.5s: should be ~50 */
    NQ_FE(nq_anim_float_value(&a), 50.0f);
    NQ_ASSERT_EQ(nq_anim_float_done(&a), 0);
}

static void test_quad_in_curves_slower_at_start(void) {
    /* quad_in at t=0.5 = 0.25 (not 0.5 like linear). Validate this is
     * distinct from linear so we know the ease is being applied. */
    NqAnimFloat a;
    nq_anim_float_init(&a, 0.0f, 100.0f, 1.0f, NQ_EASE_QUAD_IN);
    (void)nq_anim_float_update(&a, 0.5f);
    float v = nq_anim_float_value(&a);
    NQ_ASSERT(v < 50.0f - NQ_EPS);   /* quad_in at 0.5 < linear at 0.5 (=50) */
    NQ_ASSERT(v > 0.0f + NQ_EPS);    /* but > 0 */
}

static void test_zero_duration_clamps_and_completes(void) {
    NqAnimFloat a;
    nq_anim_float_init(&a, 0.0f, 50.0f, 0.0f, NQ_EASE_LINEAR);
    /* duration clamped to ~1ms internally to avoid div-by-zero */
    NQ_ASSERT_EQ(nq_anim_float_done(&a), 0);
    /* First update of any dt >= duration should complete */
    int just = nq_anim_float_update(&a, 0.1f);
    NQ_ASSERT_EQ(just, 1);
    NQ_ASSERT_EQ(nq_anim_float_done(&a), 1);
    NQ_FE(nq_anim_float_value(&a), 50.0f);
}

static void test_negative_dt_is_zero(void) {
    NqAnimFloat a;
    nq_anim_float_init(&a, 0.0f, 100.0f, 1.0f, NQ_EASE_LINEAR);
    (void)nq_anim_float_update(&a, -0.5f);
    /* Negative dt clamped -> elapsed still 0, value still 0 */
    NQ_FE(nq_anim_float_value(&a), 0.0f);
    NQ_ASSERT_EQ(nq_anim_float_done(&a), 0);
}

static void test_restart_resets(void) {
    NqAnimFloat a;
    nq_anim_float_init(&a, 0.0f, 100.0f, 1.0f, NQ_EASE_LINEAR);
    (void)nq_anim_float_update(&a, 1.0f);  /* complete */
    NQ_ASSERT_EQ(nq_anim_float_done(&a), 1);
    nq_anim_float_restart(&a);
    NQ_ASSERT_EQ(nq_anim_float_done(&a), 0);
    NQ_FE(nq_anim_float_value(&a), 0.0f);
}

static void test_value_saturates_at_to(void) {
    NqAnimFloat a;
    nq_anim_float_init(&a, 0.0f, 100.0f, 1.0f, NQ_EASE_LINEAR);
    /* Big dt — value should clamp to to (=100), not overshoot */
    (void)nq_anim_float_update(&a, 100.0f);
    NQ_FE(nq_anim_float_value(&a), 100.0f);
}

static void test_reverse_direction(void) {
    NqAnimFloat a;
    /* from=100, to=0 — animator walks 100 -> 0 over duration. */
    nq_anim_float_init(&a, 100.0f, 0.0f, 1.0f, NQ_EASE_LINEAR);
    (void)nq_anim_float_update(&a, 0.5f);
    /* Linear at 0.5s = 50 (midpoint between 100 and 0) */
    float v = nq_anim_float_value(&a);
    NQ_ASSERT(v > 49.5f && v < 50.5f);
    NQ_ASSERT_EQ(nq_anim_float_done(&a), 0);
}

NQ_TEST_REGISTER("anim_init_starts_active",        test_init_starts_active);
NQ_TEST_REGISTER("anim_linear_progresses_to_target", test_linear_progresses_to_target);
NQ_TEST_REGISTER("anim_linear_at_half_duration",   test_linear_at_half_duration);
NQ_TEST_REGISTER("anim_quad_in_curves_slower",     test_quad_in_curves_slower_at_start);
NQ_TEST_REGISTER("anim_zero_duration_clamps",      test_zero_duration_clamps_and_completes);
NQ_TEST_REGISTER("anim_negative_dt_is_zero",       test_negative_dt_is_zero);
NQ_TEST_REGISTER("anim_restart_resets",            test_restart_resets);
NQ_TEST_REGISTER("anim_value_saturates_at_to",     test_value_saturates_at_to);
NQ_TEST_REGISTER("anim_reverse_direction",        test_reverse_direction);
