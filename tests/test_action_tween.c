#include <nq/action_tween.h>
#include "test_main.c"

static void test_tween_create_destroy(void) {
    NqAnimFloat anim;
    nq_anim_float_init(&anim, 0.0f, 100.0f, 1.0f, NQ_EASE_LINEAR);
    NqActionTween *t = nq_action_tween_create(&anim);
    NQ_ASSERT(t != NULL);
    NQ_ASSERT(nq_action_tween_action(t) != NULL);
    nq_action_tween_destroy(t);
}

static void test_tween_advances_animation(void) {
    /* 1-second linear 0->100 with default mock-clock dt=0.016.
     * After 10 ticks (~160ms), value should be ~16, action still RUNNING. */
    NqAnimFloat anim;
    nq_anim_float_init(&anim, 0.0f, 100.0f, 1.0f, NQ_EASE_LINEAR);
    NqActionTween *t = nq_action_tween_create(&anim);
    for (int i = 0; i < 10; i++) {
        NQ_ASSERT_EQ(nq_action_tween_update(t, 0.016f), NQ_ACTION_RUNNING);
    }
    /* After ~0.16s, value should be around 16 */
    NQ_ASSERT(nq_anim_float_value(&anim) > 15.0f);
    NQ_ASSERT(nq_anim_float_value(&anim) < 17.0f);
    nq_action_tween_destroy(t);
}

static void test_tween_completes_when_anim_done(void) {
    /* 0.1s duration linear 0->10. After 0.2s of ticks, anim is done,
     * action must report FINISHED. */
    NqAnimFloat anim;
    nq_anim_float_init(&anim, 0.0f, 10.0f, 0.1f, NQ_EASE_LINEAR);
    NqActionTween *t = nq_action_tween_create(&anim);
    for (int i = 0; i < 5; i++) {
        (void)nq_action_tween_update(t, 0.05f);  /* 0.05 * 5 = 0.25s */
    }
    NQ_ASSERT_EQ(nq_action_tween_update(t, 0.0f), NQ_ACTION_FINISHED);
    NQ_ASSERT(nq_anim_float_done(&anim));
    nq_action_tween_destroy(t);
}

static void test_tween_with_ease_quad_in(void) {
    /* Quad-in at t=0.5 = 0.25. So a 0..100 quad-in over 1s should be ~25
     * halfway through (slower than linear's 50). */
    NqAnimFloat anim;
    nq_anim_float_init(&anim, 0.0f, 100.0f, 1.0f, NQ_EASE_QUAD_IN);
    NqActionTween *t = nq_action_tween_create(&anim);
    /* 10 ticks of 0.05 = 0.5s total */
    for (int i = 0; i < 10; i++) {
        (void)nq_action_tween_update(t, 0.05f);
    }
    float v = nq_anim_float_value(&anim);
    NQ_ASSERT(v < 30.0f);  /* quad_in at t=0.5 should be ~25 */
    NQ_ASSERT(v > 20.0f);
    nq_action_tween_destroy(t);
}

static void test_tween_null_safe(void) {
    /* NULL anim → NULL action (constructor rejects NULL anim) */
    NQ_ASSERT(nq_action_tween_create(NULL) == NULL);
    /* NULL wrapper for destroy / update is safe */
    nq_action_tween_destroy(NULL);
    NQ_ASSERT_EQ(nq_action_tween_update(NULL, 0.016f), NQ_ACTION_FINISHED);
    NQ_ASSERT(nq_action_tween_action(NULL) == NULL);
}

NQ_TEST_REGISTER("tween_create_destroy",          test_tween_create_destroy);
NQ_TEST_REGISTER("tween_advances_animation",      test_tween_advances_animation);
NQ_TEST_REGISTER("tween_completes_when_done",     test_tween_completes_when_anim_done);
NQ_TEST_REGISTER("tween_with_ease_quad_in",       test_tween_with_ease_quad_in);
NQ_TEST_REGISTER("tween_null_safe",                test_tween_null_safe);
