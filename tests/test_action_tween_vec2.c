#include "nq/action_tween_vec2.h"
#include "test_main.c"

typedef struct { int ticks; int target_ticks; } TickCtx;

static NqActionState vec2_counter_tick(NqAction *a, float dt, void *user) {
    (void)a; (void)dt;
    TickCtx *c = (TickCtx *)user;
    if (!c) return NQ_ACTION_FINISHED;
    c->ticks++;
    return c->ticks >= c->target_ticks ? NQ_ACTION_FINISHED : NQ_ACTION_RUNNING;
}

static void test_create_destroy(void) {
    TickCtx ctx = {0, 5};
    NqActionTweenVec2 *t = nq_action_tween_vec2_create(vec2_counter_tick, &ctx);
    NQ_ASSERT(t != NULL);
    NQ_ASSERT(nq_action_tween_vec2_action(t) != NULL);
    nq_action_tween_vec2_destroy(t);
}

static void test_advances(void) {
    TickCtx ctx = {0, 5};
    NqActionTweenVec2 *t = nq_action_tween_vec2_create(vec2_counter_tick, &ctx);
    for (int i = 0; i < 4; i++) {
        NQ_ASSERT_EQ(nq_action_tween_vec2_update(t, 0.016f), NQ_ACTION_RUNNING);
    }
    NQ_ASSERT_EQ(ctx.ticks, 4);
    nq_action_tween_vec2_destroy(t);
}

static void test_completes_when_callback_returns_finished(void) {
    /* target = 3, drive 5 calls → 3rd tick returns FINISHED */
    TickCtx ctx = {0, 3};
    NqActionTweenVec2 *t = nq_action_tween_vec2_create(vec2_counter_tick, &ctx);
    for (int i = 0; i < 5; i++) {
        (void)nq_action_tween_vec2_update(t, 0.016f);
    }
    NQ_ASSERT_EQ(nq_action_tween_vec2_update(t, 0.0f), NQ_ACTION_FINISHED);
    nq_action_tween_vec2_destroy(t);
}

static void test_destroy_does_not_touch_user(void) {
    TickCtx ctx = {0, 5};
    NqActionTweenVec2 *t = nq_action_tween_vec2_create(vec2_counter_tick, &ctx);
    nq_action_tween_vec2_destroy(t);
    /* ctx must still be valid */
    NQ_ASSERT_EQ(ctx.ticks, 0);
    NQ_ASSERT_EQ(ctx.target_ticks, 5);
}

static void test_null_safe(void) {
    nq_action_tween_vec2_destroy(NULL);
    NQ_ASSERT_EQ(nq_action_tween_vec2_update(NULL, 0.016f), NQ_ACTION_FINISHED);
    NQ_ASSERT(nq_action_tween_vec2_action(NULL) == NULL);
    /* NULL callback is valid: an animator can be attached afterwards. */
    NqActionTweenVec2 *t = nq_action_tween_vec2_create(NULL, NULL);
    NQ_ASSERT(t != NULL);
    nq_action_tween_vec2_destroy(t);
}

NQ_TEST_REGISTER("tween_vec2_create_destroy",     test_create_destroy)
NQ_TEST_REGISTER("tween_vec2_advances",           test_advances)
NQ_TEST_REGISTER("tween_vec2_completes",          test_completes_when_callback_returns_finished)
NQ_TEST_REGISTER("tween_vec2_no_touch_user",      test_destroy_does_not_touch_user)
NQ_TEST_REGISTER("tween_vec2_null_safe",          test_null_safe)
