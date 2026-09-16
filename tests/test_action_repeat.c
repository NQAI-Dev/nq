#include "nq/action_repeat.h"
#include "test_main.c"

/* Counter-driven action: ticks N times then FINISHED on Nth tick.
 * Suitable for repeat testing because each invocation starts fresh. */
typedef struct { int target; int ticks; } TickCtx;
static NqActionState counter_tick(NqAction *a, float dt, void *user) {
    (void)a; (void)dt;
    TickCtx *c = (TickCtx *)user;
    if (!c) return NQ_ACTION_FINISHED;
    c->ticks++;
    return c->ticks >= c->target ? NQ_ACTION_FINISHED : NQ_ACTION_RUNNING;
}

static void test_repeat_zero_iterations(void) {
    /* times=0 → FINISHED immediately on first tick. */
    TickCtx c = {1, 0};
    NqAction *sub = nq_action_create(counter_tick, NULL, &c);
    NqActionRepeat *r = nq_action_repeat_create(sub, 0);
    NQ_ASSERT(r != NULL);
    NQ_ASSERT_EQ(nq_action_repeat_update(r, 0.016f), NQ_ACTION_FINISHED);
    nq_action_repeat_destroy(r);
    nq_action_destroy(sub);
}

static void test_repeat_three_iterations(void) {
    /* times=3 — three complete iterations of a 1-tick-each sub. */
    TickCtx c = {1, 0};
    NqAction *sub = nq_action_create(counter_tick, NULL, &c);
    NqActionRepeat *r = nq_action_repeat_create(sub, 3);
    NQ_ASSERT(r != NULL);
    NQ_ASSERT_EQ(nq_action_repeat_remaining(r), 3);

    /* First iteration: tick → sub finishes → remaining=2 → tick again. */
    NQ_ASSERT_EQ(nq_action_repeat_update(r, 0.016f), NQ_ACTION_RUNNING);
    NQ_ASSERT(nq_action_repeat_remaining(r) <= 2);
    /* Same call still RUNNING because we just restarted. */

    /* Drive more ticks until the wrapper reports FINISHED. */
    for (int safety = 0; safety < 20; safety++) {
        if (nq_action_repeat_update(r, 0.016f) == NQ_ACTION_FINISHED) break;
    }
    /* After 3 iterations, the sub's total ticks = 3. */
    NQ_ASSERT_EQ(c.ticks, 3);
    /* Wrapper is FINISHED. */
    NQ_ASSERT(nq_action_repeat_remaining(r) <= 0);

    nq_action_repeat_destroy(r);
    nq_action_destroy(sub);
}

static void test_repeat_forever_doesnt_finish(void) {
    /* Infinite mode — the loop is driven externally; force-iterate
     * many times and verify the wrapper never produces FINISHED on its
     * own (sub keeps returning FINISHED-but-restart). Note: as noted
     * in src/nq_action_repeat.c, infinite mode requires the sub's tick
     * to be re-callable; counter_tick satisfies this. */
    TickCtx c = {1, 0};
    NqAction *sub = nq_action_create(counter_tick, NULL, &c);
    NqActionRepeat *r = nq_action_repeat_forever_create(sub);
    NQ_ASSERT(r != NULL);
    NQ_ASSERT_EQ(nq_action_repeat_remaining(r), -1);

    /* Drive 10 ticks; wrapper never goes FINISHED. */
    int finished_count = 0;
    for (int i = 0; i < 10; i++) {
        if (nq_action_repeat_update(r, 0.016f) == NQ_ACTION_FINISHED) {
            finished_count++;
        }
    }
    NQ_ASSERT_EQ(finished_count, 0);
    NQ_ASSERT(c.ticks >= 10);

    nq_action_repeat_destroy(r);
    nq_action_destroy(sub);
}

static void test_repeat_destroy_does_not_touch_sub(void) {
    /* repeat_destroy must not destroy the sub — caller owns it. */
    TickCtx c = {1, 0};
    NqAction *sub = nq_action_create(counter_tick, NULL, &c);
    NqActionRepeat *r = nq_action_repeat_create(sub, 2);
    nq_action_repeat_destroy(r);
    /* Sub must still be valid (if destroy() killed it, the next call
     * would crash or trigger ASAN). */
    NQ_ASSERT_EQ(nq_action_state(sub), NQ_ACTION_FINISHED);
    nq_action_destroy(sub);  /* explicit */
}

static void test_repeat_null_safe(void) {
    nq_action_repeat_destroy(NULL);
    NQ_ASSERT_EQ(nq_action_repeat_update(NULL, 0.016f), NQ_ACTION_FINISHED);
    NQ_ASSERT(nq_action_repeat_action(NULL) == NULL);
    NQ_ASSERT_EQ(nq_action_repeat_remaining(NULL), 0);
    NQ_ASSERT(nq_action_repeat_create(NULL, 1) == NULL);
    NQ_ASSERT(nq_action_repeat_forever_create(NULL) == NULL);
}

NQ_TEST_REGISTER("repeat_zero_iterations",            test_repeat_zero_iterations);
NQ_TEST_REGISTER("repeat_three_iterations",          test_repeat_three_iterations);
NQ_TEST_REGISTER("repeat_forever_doesnt_finish",     test_repeat_forever_doesnt_finish);
NQ_TEST_REGISTER("repeat_destroy_no_sub",            test_repeat_destroy_does_not_touch_sub);
NQ_TEST_REGISTER("repeat_null_safe",                 test_repeat_null_safe);

static void test_repeat_total_returns_create_count(void) {
    /* total() exposes the original count the repeat was created with */
    TickCtx ctx = {0, 1};
    NqAction *sub = nq_action_create(counter_tick, NULL, &ctx);
    NQ_ASSERT_EQ(sub, (NqAction *)sub);
    NqActionRepeat *r = nq_action_repeat_create(sub, 5);
    NQ_ASSERT_EQ(nq_action_repeat_total(r), 5);
    /* remaining() drops toward zero as ticks run, total() stays at 5 */
    (void)nq_action_repeat_update(r, 0.016f);
    NQ_ASSERT_EQ(nq_action_repeat_remaining(r), 4);
    NQ_ASSERT_EQ(nq_action_repeat_total(r), 5);
    nq_action_repeat_destroy(r);
    nq_action_destroy(sub);
}

static void test_repeat_total_forever_returns_minus_one(void) {
    /* For infinite repeats, total() returns -1 (the "no count" sentinel). */
    TickCtx ctx = {0, 1};
    NqAction *sub = nq_action_create(counter_tick, NULL, &ctx);
    NqActionRepeat *r = nq_action_repeat_forever_create(sub);
    NQ_ASSERT_EQ(nq_action_repeat_total(r), -1);
    nq_action_repeat_destroy(r);
    nq_action_destroy(sub);
}

static void test_repeat_total_null_safe(void) {
    NQ_ASSERT_EQ(nq_action_repeat_total(NULL), 0);
}

NQ_TEST_REGISTER("repeat_total_5",              test_repeat_total_returns_create_count);
NQ_TEST_REGISTER("repeat_total_forever_minus1", test_repeat_total_forever_returns_minus_one);
NQ_TEST_REGISTER("repeat_total_null_safe",      test_repeat_total_null_safe);
