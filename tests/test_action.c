#include <nq/action.h>
#include "test_main.c"

typedef struct {
    int tick_calls;
    int done_calls;
    int max_ticks;          /* when reached, returns FINISHED */
    int emit_cancelled;     /* if non-zero, action returns CANCELLED */
} CounterCtx;

/* Tick callback: bumps tick_calls, decides what to return based on ctx. */
static NqActionState counter_tick(NqAction *a, float dt, void *user) {
    (void)a; (void)dt;
    CounterCtx *c = (CounterCtx *)user;
    if (!c) return NQ_ACTION_FINISHED;
    c->tick_calls++;
    if (c->emit_cancelled) return NQ_ACTION_CANCELLED;
    return c->tick_calls >= c->max_ticks ? NQ_ACTION_FINISHED
                                          : NQ_ACTION_RUNNING;
}

/* Done callback: bumps done_calls. */
static void counter_done(NqAction *a, void *user) {
    (void)a;
    CounterCtx *c = (CounterCtx *)user;
    if (c) c->done_calls++;
}

static void test_action_create_starts_running(void) {
    CounterCtx ctx = {0, 0, 100, 0};
    NqAction *a = nq_action_create(counter_tick, counter_done, &ctx);
    NQ_ASSERT(a != NULL);
    NQ_ASSERT(nq_action_state(a) == NQ_ACTION_RUNNING);
    NQ_ASSERT(!nq_action_is_finished(a));
    nq_action_destroy(a);
}

static void test_action_tick_drives_state(void) {
    CounterCtx ctx = {0, 0, 3, 0};  /* finishes on tick #3 */
    NqAction *a = nq_action_create(counter_tick, counter_done, &ctx);

    NQ_ASSERT(nq_action_update(a, 0.016f) == NQ_ACTION_RUNNING);
    NQ_ASSERT_EQ(ctx.tick_calls, 1);
    NQ_ASSERT(nq_action_state(a) == NQ_ACTION_RUNNING);

    NQ_ASSERT(nq_action_update(a, 0.016f) == NQ_ACTION_RUNNING);
    NQ_ASSERT_EQ(ctx.tick_calls, 2);

    /* Third tick — counter returns FINISHED, action becomes FINISHED. */
    NQ_ASSERT(nq_action_update(a, 0.016f) == NQ_ACTION_FINISHED);
    NQ_ASSERT_EQ(ctx.tick_calls, 3);
    NQ_ASSERT(nq_action_is_finished(a));
    nq_action_destroy(a);
}

static void test_action_done_fires_once(void) {
    CounterCtx ctx = {0, 0, 2, 0};
    NqAction *a = nq_action_create(counter_tick, counter_done, &ctx);

    nq_action_update(a, 0.016f);
    NQ_ASSERT_EQ(ctx.done_calls, 0);  /* still RUNNING */
    nq_action_update(a, 0.016f);
    NQ_ASSERT_EQ(ctx.done_calls, 1);  /* FINISHED reached */

    /* Further updates don't re-fire done — done_fired flag. */
    nq_action_update(a, 0.016f);
    nq_action_update(a, 0.016f);
    NQ_ASSERT_EQ(ctx.done_calls, 1);
    nq_action_destroy(a);
}

static void test_action_cancel_fires_done_with_cancelled_state(void) {
    CounterCtx ctx = {0, 0, 100, 0};
    NqAction *a = nq_action_create(counter_tick, counter_done, &ctx);

    nq_action_cancel(a);
    NQ_ASSERT_EQ(ctx.done_calls, 1);
    NQ_ASSERT(nq_action_state(a) == NQ_ACTION_CANCELLED);
    NQ_ASSERT(nq_action_is_finished(a));

    /* Idempotent: double cancel doesn't double-fire done. */
    nq_action_cancel(a);
    NQ_ASSERT_EQ(ctx.done_calls, 1);

    /* Subsequent updates don't re-tick once cancelled. */
    NQ_ASSERT_EQ(ctx.tick_calls, 0);
    nq_action_update(a, 0.016f);
    NQ_ASSERT_EQ(ctx.tick_calls, 0);
    nq_action_destroy(a);
}

static void test_action_tick_returns_cancelled_propagates(void) {
    /* If the tick callback itself returns NQ_ACTION_CANCELLED (e.g. it
     * detects an external signal), the action must stop calling done()
     * — cancellation is the caller's responsibility, not done()'s. */
    CounterCtx ctx = {0, 0, 100, 1};  /* emit_cancelled = 1 */
    NqAction *a = nq_action_create(counter_tick, counter_done, &ctx);
    NQ_ASSERT(nq_action_update(a, 0.016f) == NQ_ACTION_CANCELLED);
    NQ_ASSERT(nq_action_state(a) == NQ_ACTION_CANCELLED);
    NQ_ASSERT_EQ(ctx.done_calls, 0);  /* cancelled from tick: no done */
    nq_action_destroy(a);
}

static void test_action_null_safe(void) {
    /* All entry points on NULL input. */
    nq_action_destroy(NULL);
    NQ_ASSERT(nq_action_update(NULL, 0.016f) == NQ_ACTION_FINISHED);
    nq_action_cancel(NULL);
    NQ_ASSERT(nq_action_state(NULL) == NQ_ACTION_FINISHED);
    NQ_ASSERT(nq_action_is_finished(NULL));
    /* create with NULL tick must return NULL (tick is mandatory). */
    NQ_ASSERT(nq_action_create(NULL, NULL, NULL) == NULL);
    /* create with non-NULL tick + NULL done is allowed. */
    NqAction *a = nq_action_create(counter_tick, NULL, NULL);
    NQ_ASSERT(a != NULL);
    NQ_ASSERT(nq_action_state(a) == NQ_ACTION_RUNNING);
    nq_action_destroy(a);
}

static void test_action_finished_does_not_advance(void) {
    /* Once FINISHED, further updates leave the action state alone —
     * the tick callback is NOT re-invoked. */
    CounterCtx ctx = {0, 0, 2, 0};
    NqAction *a = nq_action_create(counter_tick, counter_done, &ctx);
    nq_action_update(a, 0.016f);  /* RUNNING */
    nq_action_update(a, 0.016f);  /* FINISHED */
    NQ_ASSERT_EQ(ctx.tick_calls, 2);
    nq_action_update(a, 0.016f);
    nq_action_update(a, 0.016f);
    nq_action_update(a, 0.016f);
    NQ_ASSERT_EQ(ctx.tick_calls, 2);  /* unchanged */
    nq_action_destroy(a);
}

NQ_TEST_REGISTER("action_create_running",         test_action_create_starts_running)
NQ_TEST_REGISTER("action_tick_drives_state",     test_action_tick_drives_state)
NQ_TEST_REGISTER("action_done_fires_once",        test_action_done_fires_once)
NQ_TEST_REGISTER("action_cancel_state",          test_action_cancel_fires_done_with_cancelled_state)
NQ_TEST_REGISTER("action_tick_cancelled",        test_action_tick_returns_cancelled_propagates)
NQ_TEST_REGISTER("action_null_safe",             test_action_null_safe)
NQ_TEST_REGISTER("action_finished_static",       test_action_finished_does_not_advance)
