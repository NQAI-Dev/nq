#include "nq/anim_color.h"
#include "test_main.c"

static void test_init_starts_active(void) {
    NqAnimColor a;
    nq_anim_color_init(&a,
        NQ_COLOR_RGB(0, 0, 0), NQ_COLOR_RGB(255, 255, 255),
        1.0f, NQ_EASE_LINEAR);
    NQ_ASSERT(nq_anim_color_done(&a) == 0);
    /* Value at t=0 should be the `from` color (RGB 0,0,0). */
    NqColor v = nq_anim_color_value(&a);
    NQ_ASSERT_EQ(v.r, 0);
    NQ_ASSERT_EQ(v.g, 0);
    NQ_ASSERT_EQ(v.b, 0);
    NQ_ASSERT_EQ(v.a, 255);
}

static void test_linear_progresses_to_target(void) {
    /* 1-second linear 0..255 white-from-black. After 1s tick → done, value = white. */
    NqAnimColor a;
    nq_anim_color_init(&a,
        NQ_COLOR_RGB(0, 0, 0), NQ_COLOR_RGB(255, 255, 255),
        1.0f, NQ_EASE_LINEAR);
    /* Big dt — should complete. */
    NQ_ASSERT_EQ(nq_anim_color_update(&a, 1.0f), 1);
    NQ_ASSERT(nq_anim_color_done(&a));
    NqColor v = nq_anim_color_value(&a);
    NQ_ASSERT_EQ(v.r, 255);
    NQ_ASSERT_EQ(v.g, 255);
    NQ_ASSERT_EQ(v.b, 255);
}

static void test_linear_midpoint(void) {
    /* 1-second linear black→white. After 0.5s, channels should be ~127/128. */
    NqAnimColor a;
    nq_anim_color_init(&a,
        NQ_COLOR_RGB(0, 0, 0), NQ_COLOR_RGB(255, 255, 255),
        1.0f, NQ_EASE_LINEAR);
    NQ_ASSERT_EQ(nq_anim_color_update(&a, 0.5f), 0);
    NqColor v = nq_anim_color_value(&a);
    NQ_ASSERT(v.r >= 127 && v.r <= 128);
    NQ_ASSERT(v.g >= 127 && v.g <= 128);
    NQ_ASSERT(v.b >= 127 && v.b <= 128);
}

static void test_saturates_at_to(void) {
    /* Big dt — value clamps at `to`, no overshoot. */
    NqAnimColor a;
    nq_anim_color_init(&a,
        NQ_COLOR_RGB(10, 20, 30), NQ_COLOR_RGB(100, 150, 200),
        1.0f, NQ_EASE_LINEAR);
    nq_anim_color_update(&a, 100.0f);
    NqColor v = nq_anim_color_value(&a);
    NQ_ASSERT_EQ(v.r, 100);
    NQ_ASSERT_EQ(v.g, 150);
    NQ_ASSERT_EQ(v.b, 200);
}

static void test_reverse_direction(void) {
    /* from=white, to=black → reverse flag set → value walks from to (black)
     * at t=0 toward from (white) as t→1. Wait, that's wrong; let me
     * re-check the contract. With reverse=1:
     *   value(t=0) = lerp(to, from, ease(0)) = to
     *   value(t=1) = lerp(to, from, ease(1)) = from
     * So reverse means "start at to, end at from". That's the inverse of
     * what most users expect; the only way `from > to` would normally
     * arise is when the user constructs the animator with explicit
     * "from=current, to=target" and the values happen to descend.
     * Either way: at t=0 the value should be `to` (the starting point). */
    NqAnimColor a;
    nq_anim_color_init(&a,
        NQ_COLOR_RGB(255, 255, 255), NQ_COLOR_RGB(0, 0, 0),
        1.0f, NQ_EASE_LINEAR);
    NqColor v0 = nq_anim_color_value(&a);
    NQ_ASSERT_EQ(v0.r, 0);  /* reverse → value at t=0 is `to` = black */
    /* Drive to completion; final value should be `from` = white. */
    (void)nq_anim_color_update(&a, 1.0f);
    NqColor v1 = nq_anim_color_value(&a);
    NQ_ASSERT_EQ(v1.r, 255);
}

static void test_zero_duration_clamped(void) {
    NqAnimColor a;
    nq_anim_color_init(&a,
        NQ_COLOR_RGB(0, 0, 0), NQ_COLOR_RGB(255, 255, 255),
        0.0f, NQ_EASE_LINEAR);
    /* First update of any positive dt must complete. */
    NQ_ASSERT_EQ(nq_anim_color_update(&a, 0.016f), 1);
    NQ_ASSERT(nq_anim_color_done(&a));
}

static void test_negative_dt_clamped(void) {
    NqAnimColor a;
    nq_anim_color_init(&a,
        NQ_COLOR_RGB(0, 0, 0), NQ_COLOR_RGB(255, 255, 255),
        1.0f, NQ_EASE_LINEAR);
    (void)nq_anim_color_update(&a, -0.5f);
    NqColor v = nq_anim_color_value(&a);
    NQ_ASSERT_EQ(v.r, 0);  /* still at `from`, elapsed clamped to 0 */
}

static void test_restart_resets(void) {
    NqAnimColor a;
    nq_anim_color_init(&a,
        NQ_COLOR_RGB(0, 0, 0), NQ_COLOR_RGB(255, 255, 255),
        1.0f, NQ_EASE_LINEAR);
    nq_anim_color_update(&a, 1.0f);  /* completes */
    NQ_ASSERT(nq_anim_color_done(&a));
    nq_anim_color_restart(&a);
    NQ_ASSERT(nq_anim_color_done(&a) == 0);
    NqColor v = nq_anim_color_value(&a);
    NQ_ASSERT_EQ(v.r, 0);  /* back to `from` */
}

static void test_alpha_animates_independently(void) {
    /* from=opaque red, to=transparent red, linear over 1s. */
    NqAnimColor a;
    nq_anim_color_init(&a,
        NQ_COLOR_RGBA(255, 0, 0, 255),
        NQ_COLOR_RGBA(255, 0,   0,   0),
        1.0f, NQ_EASE_LINEAR);
    (void)nq_anim_color_update(&a, 0.5f);
    NqColor v = nq_anim_color_value(&a);
    NQ_ASSERT_EQ(v.r, 255);
    /* alpha mid: 127 or 128 */
    NQ_ASSERT(v.a >= 127 && v.a <= 128);
}

static void test_null_safe(void) {
    nq_anim_color_init(NULL, NQ_COLOR_RGB(0,0,0), NQ_COLOR_RGB(0,0,0),
                      1.0f, NQ_EASE_LINEAR);
    NQ_ASSERT_EQ(nq_anim_color_update(NULL, 0.016f), 0);
    NQ_ASSERT(nq_anim_color_done(NULL));
    NQ_ASSERT_EQ(nq_anim_color_value(NULL).r, 0);
    nq_anim_color_restart(NULL);
    /* Reaching here means no crash. */
    NQ_ASSERT(1);
}

NQ_TEST_REGISTER("anim_color_init_starts_active",  test_init_starts_active);
NQ_TEST_REGISTER("anim_color_linear_progresses",   test_linear_progresses_to_target);
NQ_TEST_REGISTER("anim_color_linear_midpoint",      test_linear_midpoint);
NQ_TEST_REGISTER("anim_color_saturates",            test_saturates_at_to);
NQ_TEST_REGISTER("anim_color_reverse_direction",    test_reverse_direction);
NQ_TEST_REGISTER("anim_color_zero_duration",        test_zero_duration_clamped);
NQ_TEST_REGISTER("anim_color_negative_dt",         test_negative_dt_clamped);
NQ_TEST_REGISTER("anim_color_restart_resets",     test_restart_resets);
NQ_TEST_REGISTER("anim_color_alpha",               test_alpha_animates_independently);
NQ_TEST_REGISTER("anim_color_null_safe",          test_null_safe);
