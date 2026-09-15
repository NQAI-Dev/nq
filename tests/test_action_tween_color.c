#include "nq_action_tween_color.h"
#include "test_main.c"

static void test_create_destroy(void) {
    NqAnimColor anim;
    nq_anim_color_init(&anim,
        NQ_COLOR_RGB(0, 0, 0), NQ_COLOR_RGB(255, 255, 255),
        1.0f, NQ_EASE_LINEAR);
    NqActionTweenColor *t = nq_action_tween_color_create(&anim);
    NQ_ASSERT(t != NULL);
    NQ_ASSERT(nq_action_tween_color_action(t) != NULL);
    nq_action_tween_color_destroy(t);
}

static void test_advances_anim(void) {
    /* 1-second linear black→white. After ~0.16s of 16ms ticks, the R
     * channel should be somewhere in the 30-50 range (not yet near 255). */
    NqAnimColor anim;
    nq_anim_color_init(&anim,
        NQ_COLOR_RGB(0, 0, 0), NQ_COLOR_RGB(255, 255, 255),
        1.0f, NQ_EASE_LINEAR);
    NqActionTweenColor *t = nq_action_tween_color_create(&anim);
    for (int i = 0; i < 10; i++) {
        NQ_ASSERT_EQ(nq_action_tween_color_update(t, 0.016f), NQ_ACTION_RUNNING);
    }
    NqColor v = nq_anim_color_value(&anim);
    NQ_ASSERT(v.r > 0);
    NQ_ASSERT(v.r < 100);  /* shouldn't be near 255 yet */
    nq_action_tween_color_destroy(t);
}

static void test_completes_when_anim_done(void) {
    /* 0.1s linear. Drive 5x0.05s → anim completes → action FINISHED. */
    NqAnimColor anim;
    nq_anim_color_init(&anim,
        NQ_COLOR_RGB(0, 0, 0), NQ_COLOR_RGB(255, 255, 255),
        0.1f, NQ_EASE_LINEAR);
    NqActionTweenColor *t = nq_action_tween_color_create(&anim);
    for (int i = 0; i < 4; i++) {
        (void)nq_action_tween_color_update(t, 0.05f);
    }
    NQ_ASSERT_EQ(nq_action_tween_color_update(t, 0.0f), NQ_ACTION_FINISHED);
    NQ_ASSERT(nq_anim_color_done(&anim));
    nq_action_tween_color_destroy(t);
}

static void test_with_ease_quad_in(void) {
    /* quad_in at t=0.5 ≈ 0.25 — so the lerp channel should be < 128
     * (linear midpoint), not 128. */
    NqAnimColor anim;
    nq_anim_color_init(&anim,
        NQ_COLOR_RGB(0, 0, 0), NQ_COLOR_RGB(255, 255, 255),
        1.0f, NQ_EASE_QUAD_IN);
    NqActionTweenColor *t = nq_action_tween_color_create(&anim);
    /* 10 ticks of 0.05 = 0.5s elapsed */
    for (int i = 0; i < 10; i++) {
        (void)nq_action_tween_color_update(t, 0.05f);
    }
    NqColor v = nq_anim_color_value(&anim);
    NQ_ASSERT(v.r < 80);   /* quad_in at t=0.5 → ~64; allow headroom */
    NQ_ASSERT(v.r > 0);
    nq_action_tween_color_destroy(t);
}

static void test_null_safe(void) {
    /* NULL anim → NULL action (constructor rejects) */
    NQ_ASSERT(nq_action_tween_color_create(NULL) == NULL);
    /* NULL wrapper for destroy / update is safe */
    nq_action_tween_color_destroy(NULL);
    NQ_ASSERT_EQ(nq_action_tween_color_update(NULL, 0.016f), NQ_ACTION_FINISHED);
    NQ_ASSERT(nq_action_tween_color_action(NULL) == NULL);
}

static void test_destroy_does_not_touch_anim(void) {
    /* destroy must not free the anim — caller owns it. */
    NqAnimColor anim;
    nq_anim_color_init(&anim,
        NQ_COLOR_RGB(0, 0, 0), NQ_COLOR_RGB(255, 255, 255),
        1.0f, NQ_EASE_LINEAR);
    NqActionTweenColor *t = nq_action_tween_color_create(&anim);
    nq_action_tween_color_destroy(t);
    /* Anim must still be valid (if destroy() killed it, the next call
     * would crash or trigger ASAN). */
    NQ_ASSERT_EQ(nq_anim_color_done(&anim), 0);
    nq_anim_color_destroy(&anim);  /* not a thing; just sanity */
}

NQ_TEST_REGISTER("tween_color_create_destroy",    test_create_destroy);
NQ_TEST_REGISTER("tween_color_advances_anim",     test_advances_anim);
NQ_TEST_REGISTER("tween_color_completes",        test_completes_when_anim_done);
NQ_TEST_REGISTER("tween_color_with_ease",        test_with_ease_quad_in);
NQ_TEST_REGISTER("tween_color_null_safe",        test_null_safe);
NQ_TEST_REGISTER("tween_color_no_touch_anim",   test_destroy_does_not_touch_anim);
