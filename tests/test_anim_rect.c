#include "nq/anim_rect.h"
#include "test_main.c"

static void test_anim_rect_init_starts_active(void) {
    NqAnimRect a;
    nq_anim_rect_init(&a, nq_rect(0,0,10,10), nq_rect(100,200,300,400),
                      1.0f, NQ_EASE_LINEAR);
    NQ_ASSERT(nq_anim_rect_done(&a) == 0);
    NqRect v = nq_anim_rect_value(&a);
    NQ_ASSERT_EQ(v.x, 0);
    NQ_ASSERT_EQ(v.y, 0);
    NQ_ASSERT_EQ(v.w, 10);
    NQ_ASSERT_EQ(v.h, 10);
}

static void test_anim_rect_linear_progresses_to_target(void) {
    /* 1-second linear (0,0,0,0) → (100,200,300,400). After 1s tick → done, value = to. */
    NqAnimRect a;
    nq_anim_rect_init(&a, nq_rect(0,0,0,0), nq_rect(100,200,300,400),
                      1.0f, NQ_EASE_LINEAR);
    NQ_ASSERT_EQ(nq_anim_rect_update(&a, 1.0f), 1);
    NQ_ASSERT(nq_anim_rect_done(&a));
    NqRect v = nq_anim_rect_value(&a);
    NQ_ASSERT_EQ(v.x, 100);
    NQ_ASSERT_EQ(v.y, 200);
    NQ_ASSERT_EQ(v.w, 300);
    NQ_ASSERT_EQ(v.h, 400);
}

static void test_anim_rect_linear_midpoint(void) {
    /* 1s linear (0,0,0,0) → (100,200,300,400). At t=0.5 every component ≈ half. */
    NqAnimRect a;
    nq_anim_rect_init(&a, nq_rect(0,0,0,0), nq_rect(100,200,300,400),
                      1.0f, NQ_EASE_LINEAR);
    NQ_ASSERT_EQ(nq_anim_rect_update(&a, 0.5f), 0);
    NqRect v = nq_anim_rect_value(&a);
    /* integer division: 100/2 = 50, 200/2 = 100, 300/2 = 150, 400/2 = 200 */
    NQ_ASSERT_EQ(v.x, 50);
    NQ_ASSERT_EQ(v.y, 100);
    NQ_ASSERT_EQ(v.w, 150);
    NQ_ASSERT_EQ(v.h, 200);
}

static void test_anim_rect_saturates_at_to(void) {
    /* Big dt → value clamps at `to`, no overshoot. */
    NqAnimRect a;
    nq_anim_rect_init(&a, nq_rect(10,20,30,40), nq_rect(100,150,200,250),
                      1.0f, NQ_EASE_LINEAR);
    nq_anim_rect_update(&a, 100.0f);
    NqRect v = nq_anim_rect_value(&a);
    NQ_ASSERT_EQ(v.x, 100);
    NQ_ASSERT_EQ(v.y, 150);
    NQ_ASSERT_EQ(v.w, 200);
    NQ_ASSERT_EQ(v.h, 250);
}

static void test_anim_rect_reverse_direction(void) {
    /* from=(200,200,100,100), to=(0,0,0,0). Reverse flag set → at t=0 value
     * is `to` (the starting point), at t=1 value is `from` (the end). */
    NqAnimRect a;
    nq_anim_rect_init(&a, nq_rect(200,200,100,100), nq_rect(0,0,0,0),
                      1.0f, NQ_EASE_LINEAR);
    NqRect v0 = nq_anim_rect_value(&a);
    NQ_ASSERT_EQ(v0.x, 0);  /* reverse → at t=0 value is `to` */
    nq_anim_rect_update(&a, 1.0f);
    NqRect v1 = nq_anim_rect_value(&a);
    NQ_ASSERT_EQ(v1.x, 200);  /* at t=1 reaches `from` */
}

static void test_anim_rect_zero_duration_clamps(void) {
    /* duration_seconds = 0 → clamped to 0.001; first positive dt completes. */
    NqAnimRect a;
    nq_anim_rect_init(&a, nq_rect(0,0,0,0), nq_rect(100,200,300,400),
                      0.0f, NQ_EASE_LINEAR);
    NQ_ASSERT_EQ(nq_anim_rect_update(&a, 0.016f), 1);
    NQ_ASSERT(nq_anim_rect_done(&a));
}

static void test_anim_rect_negative_dt_clamped(void) {
    /* Negative dt doesn't rewind elapsed time. */
    NqAnimRect a;
    nq_anim_rect_init(&a, nq_rect(0,0,0,0), nq_rect(100,200,300,400),
                      1.0f, NQ_EASE_LINEAR);
    nq_anim_rect_update(&a, -0.5f);
    NqRect v = nq_anim_rect_value(&a);
    NQ_ASSERT_EQ(v.x, 0);  /* still at `from`, elapsed clamped to 0 */
}

static void test_anim_rect_restart_resets(void) {
    NqAnimRect a;
    nq_anim_rect_init(&a, nq_rect(0,0,0,0), nq_rect(100,200,300,400),
                      1.0f, NQ_EASE_LINEAR);
    nq_anim_rect_update(&a, 1.0f);
    NQ_ASSERT(nq_anim_rect_done(&a));
    nq_anim_rect_restart(&a);
    NQ_ASSERT(nq_anim_rect_done(&a) == 0);
    NqRect v = nq_anim_rect_value(&a);
    NQ_ASSERT_EQ(v.x, 0);  /* back to `from` */
}

static void test_anim_rect_null_safe(void) {
    nq_anim_rect_init(NULL, nq_rect(0,0,0,0), nq_rect(0,0,0,0),
                     1.0f, NQ_EASE_LINEAR);
    NQ_ASSERT_EQ(nq_anim_rect_update(NULL, 0.016f), 0);
    NQ_ASSERT(nq_anim_rect_done(NULL));
    NQ_ASSERT_EQ(nq_anim_rect_value(NULL).x, 0);
    NqRect v = nq_anim_rect_value(NULL);
    NQ_ASSERT_EQ(v.x, 0);
    nq_anim_rect_restart(NULL);
}

NQ_TEST_REGISTER("anim_rect_init_starts_active", test_anim_rect_init_starts_active);
NQ_TEST_REGISTER("anim_rect_linear_progresses", test_anim_rect_linear_progresses_to_target);
NQ_TEST_REGISTER("anim_rect_linear_midpoint",    test_anim_rect_linear_midpoint);
NQ_TEST_REGISTER("anim_rect_saturates",          test_anim_rect_saturates_at_to);
NQ_TEST_REGISTER("anim_rect_reverse_direction", test_anim_rect_reverse_direction);
NQ_TEST_REGISTER("anim_rect_zero_duration",     test_anim_rect_zero_duration_clamps);
NQ_TEST_REGISTER("anim_rect_negative_dt",         test_anim_rect_negative_dt_clamped);
NQ_TEST_REGISTER("anim_rect_restart_resets",     test_anim_rect_restart_resets);
NQ_TEST_REGISTER("anim_rect_null_safe",          test_anim_rect_null_safe);
