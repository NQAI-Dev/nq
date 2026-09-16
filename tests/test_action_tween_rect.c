#include "nq/action_tween_rect.h"
#include "test_main.c"

/* User-supplied state: interpolates between two rects over 1 second. */
typedef struct { NqRect from; NqRect to; int ticks; int target_ticks; } RectCtx;

static NqActionState rect_tick_advance(NqAction *a, float dt, void *user) {
    (void)a;
    RectCtx *c = (RectCtx *)user;
    if (!c) return NQ_ACTION_FINISHED;
    c->ticks++;
    return c->ticks >= c->target_ticks ? NQ_ACTION_FINISHED : NQ_ACTION_RUNNING;
}

static void test_create_destroy(void) {
    RectCtx ctx = {{0,0,0,0}, {100,100,100,100}, 0, 5};
    NqActionTweenRect *t = nq_action_tween_rect_create(rect_tick_advance, &ctx);
    NQ_ASSERT(t != NULL);
    NQ_ASSERT(nq_action_tween_rect_action(t) != NULL);
    nq_action_tween_rect_destroy(t);
}

static void test_advances(void) {
    RectCtx ctx = {{0,0,0,0}, {100,100,100,100}, 0, 5};
    NqActionTweenRect *t = nq_action_tween_rect_create(rect_tick_advance, &ctx);
    for (int i = 0; i < 4; i++) {
        NQ_ASSERT_EQ(nq_action_tween_rect_update(t, 0.016f), NQ_ACTION_RUNNING);
    }
    NQ_ASSERT_EQ(ctx.ticks, 4);
    nq_action_tween_rect_destroy(t);
}

static void test_completes_when_callback_returns_finished(void) {
    /* target_ticks = 3, drive 4 calls → 3rd tick returns FINISHED. */
    RectCtx ctx = {{0,0,0,0}, {100,100,100,100}, 0, 3};
    NqActionTweenRect *t = nq_action_tween_rect_create(rect_tick_advance, &ctx);
    for (int i = 0; i < 5; i++) {
        (void)nq_action_tween_rect_update(t, 0.016f);
    }
    NQ_ASSERT_EQ(nq_action_tween_rect_update(t, 0.0f), NQ_ACTION_FINISHED);
    nq_action_tween_rect_destroy(t);
}

static void test_destroy_does_not_touch_user(void) {
    RectCtx ctx = {{0,0,0,0}, {100,100,100,100}, 0, 5};
    NqActionTweenRect *t = nq_action_tween_rect_create(rect_tick_advance, &ctx);
    nq_action_tween_rect_destroy(t);
    /* ctx must still be valid */
    NQ_ASSERT_EQ(ctx.from.w, 0);
    NQ_ASSERT_EQ(ctx.to.w, 100);
}

static void test_null_safe(void) {
    nq_action_tween_rect_destroy(NULL);
    NQ_ASSERT_EQ(nq_action_tween_rect_update(NULL, 0.016f), NQ_ACTION_FINISHED);
    NQ_ASSERT(nq_action_tween_rect_action(NULL) == NULL);
    /* NULL callback (rect_tick = NULL) → constructor returns NULL */
    NQ_ASSERT(nq_action_tween_rect_create(NULL, NULL) == NULL);
}

NQ_TEST_REGISTER("tween_rect_create_destroy",     test_create_destroy);
NQ_TEST_REGISTER("tween_rect_advances",           test_advances);
NQ_TEST_REGISTER("tween_rect_completes",          test_completes_when_callback_returns_finished);
NQ_TEST_REGISTER("tween_rect_no_touch_user",      test_destroy_does_not_touch_user);
NQ_TEST_REGISTER("tween_rect_null_safe",          test_null_safe);
