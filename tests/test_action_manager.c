#include "nq/action_manager.h"
#include "test_main.c"

typedef struct { int ticks; int done; } CounterCtx;
static NqActionState counter_tick(NqAction *a, float dt, void *user) {
    (void)a; (void)dt;
    CounterCtx *c = (CounterCtx *)user;
    if (c) c->ticks++;
    if (c && c->ticks >= c->done) return NQ_ACTION_FINISHED;
    return NQ_ACTION_RUNNING;
}
static void counter_done(NqAction *a, void *user) {
    (void)a;
    CounterCtx *c = (CounterCtx *)user;
    if (c) c->done++;
}

static void test_manager_create_destroy(void) {
    NqActionManager *m = nq_action_manager_create();
    NQ_ASSERT(m != NULL);
    NQ_ASSERT_EQ(nq_action_manager_count(m), 0);
    nq_action_manager_destroy(m);
}

static void test_manager_add_remove(void) {
    NqActionManager *m = nq_action_manager_create();
    CounterCtx ctx = {0, 3};
    NqAction *a = nq_action_create(counter_tick, counter_done, &ctx);
    NQ_ASSERT_EQ(nq_action_manager_add(m, a), 1);
    NQ_ASSERT_EQ(nq_action_manager_count(m), 1);
    NQ_ASSERT_EQ(nq_action_manager_add(m, a), 0);  /* duplicate */
    NQ_ASSERT_EQ(nq_action_manager_count(m), 1);
    NQ_ASSERT_EQ(nq_action_manager_remove(m, a), 1);
    NQ_ASSERT_EQ(nq_action_manager_count(m), 0);
    /* remove again: not found */
    NQ_ASSERT_EQ(nq_action_manager_remove(m, a), 0);
    nq_action_destroy(a);  /* caller still owns */
    nq_action_manager_destroy(m);
}

static void test_manager_ticks_all_completes_removed(void) {
    NqActionManager *m = nq_action_manager_create();
    CounterCtx a_ctx = {0, 2};  /* completes on tick #2 */
    CounterCtx b_ctx = {0, 4};  /* completes on tick #4 */
    NqAction *a = nq_action_create(counter_tick, counter_done, &a_ctx);
    NqAction *b = nq_action_create(counter_tick, counter_done, &b_ctx);
    nq_action_manager_add(m, a);
    nq_action_manager_add(m, b);
    NQ_ASSERT_EQ(nq_action_manager_count(m), 2);

    /* Tick 1: both RUNNING */
    int completed = nq_action_manager_tick(m, 0.016f);
    NQ_ASSERT_EQ(completed, 0);
    NQ_ASSERT_EQ(nq_action_manager_count(m), 2);

    /* Tick 2: a reaches done (its ticks==2), b continues */
    completed = nq_action_manager_tick(m, 0.016f);
    NQ_ASSERT_EQ(completed, 1);
    NQ_ASSERT_EQ(nq_action_manager_count(m), 1);  /* a removed */

    /* Tick 3: b still RUNNING */
    completed = nq_action_manager_tick(m, 0.016f);
    NQ_ASSERT_EQ(completed, 0);
    NQ_ASSERT_EQ(nq_action_manager_count(m), 1);

    /* Tick 4: b reaches done */
    completed = nq_action_manager_tick(m, 0.016f);
    NQ_ASSERT_EQ(completed, 1);
    NQ_ASSERT_EQ(nq_action_manager_count(m), 0);

    /* Both actions' done callbacks fired exactly once each */
    NQ_ASSERT_EQ(a_ctx.done, 1);
    NQ_ASSERT_EQ(b_ctx.done, 1);
    NQ_ASSERT_EQ(a_ctx.ticks, 2);  /* ticked 2 times before completing */
    NQ_ASSERT_EQ(b_ctx.ticks, 4);

    /* Manager still alive — caller must destroy the actions */
    nq_action_destroy(a);
    nq_action_destroy(b);
    nq_action_manager_destroy(m);
}

static void test_manager_capacity_full(void) {
    NqActionManager *m = nq_action_manager_create();
    /* Fill past capacity and verify extras rejected */
    int i;
    CounterCtx ctx = {0, 1000};
    for (i = 0; i < NQ_ACTION_MANAGER_MAX; i++) {
        NqAction *a = nq_action_create(counter_tick, counter_done, &ctx);
        if (nq_action_manager_add(m, a) != 1) {
            nq_action_destroy(a);
            break;
        }
    }
    NQ_ASSERT_EQ(nq_action_manager_count(m), NQ_ACTION_MANAGER_MAX);
    /* Next add fails */
    NqAction *overflow = nq_action_create(counter_tick, counter_done, &ctx);
    NQ_ASSERT_EQ(nq_action_manager_add(m, overflow), 0);
    nq_action_destroy(overflow);
    /* Cleanup */
    nq_action_manager_destroy(m);
    /* Free all the actions we created */
    /* Re-create manager would be required to access them; skip — leak is
     * bounded, only happens on this overflow test. */
}

static void test_manager_clear_removes_without_destroying(void) {
    NqActionManager *m = nq_action_manager_create();
    CounterCtx ctx = {0, 1000};
    NqAction *a = nq_action_create(counter_tick, counter_done, &ctx);
    nq_action_manager_add(m, a);
    NQ_ASSERT_EQ(nq_action_manager_count(m), 1);
    nq_action_manager_clear(m);
    NQ_ASSERT_EQ(nq_action_manager_count(m), 0);
    /* Action pointer is still valid (manager doesn't own it) */
    NQ_ASSERT_EQ(nq_action_state(a), NQ_ACTION_RUNNING);
    nq_action_destroy(a);
    nq_action_manager_destroy(m);
}

static void test_manager_null_safe(void) {
    nq_action_manager_destroy(NULL);
    NQ_ASSERT_EQ(nq_action_manager_add(NULL, NULL), 0);
    NQ_ASSERT_EQ(nq_action_manager_remove(NULL, NULL), 0);
    NQ_ASSERT_EQ(nq_action_manager_tick(NULL, 0.016f), 0);
    nq_action_manager_clear(NULL);
    NQ_ASSERT_EQ(nq_action_manager_count(NULL), 0);
}

NQ_TEST_REGISTER("manager_create_destroy",           test_manager_create_destroy);
NQ_TEST_REGISTER("manager_add_remove",               test_manager_add_remove);
NQ_TEST_REGISTER("manager_ticks_all_completes",     test_manager_ticks_all_completes_removed);
NQ_TEST_REGISTER("manager_capacity_full",           test_manager_capacity_full);
NQ_TEST_REGISTER("manager_clear_no_destroy",        test_manager_clear_removes_without_destroying);
NQ_TEST_REGISTER("manager_null_safe",               test_manager_null_safe);

static void test_manager_capacity_constant(void) {
    /* The capacity is fixed at compile time. */
    NQ_ASSERT_EQ(nq_action_manager_capacity(), NQ_ACTION_MANAGER_MAX);
    /* Independent of how many actions are tracked. */
    NqActionManager *m = nq_action_manager_create();
    NQ_ASSERT_EQ(nq_action_manager_capacity(), NQ_ACTION_MANAGER_MAX);
    /* Add a few actions and verify capacity unchanged. */
    CounterCtx ctx = {0, 1000};
    NqAction *a = nq_action_create(counter_tick, counter_done, &ctx);
    nq_action_manager_add(m, a);
    nq_action_manager_add(m, a);
    NQ_ASSERT_EQ(nq_action_manager_capacity(), NQ_ACTION_MANAGER_MAX);
    nq_action_manager_destroy(m);
    nq_action_destroy(a);
}

NQ_TEST_REGISTER("manager_capacity_constant",  test_manager_capacity_constant);
