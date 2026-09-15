/*
 * Tests for nq_action_reset on the composition primitives that
 * install a reset_fn (delay, tween, sequence, spawn, repeat). The base
 * nq_action_test.c covers the NULL-safe paths and state machine.
 */
#include <nq/action.h>
#include <nq/action_delay.h>
#include <nq/action_tween.h>
#include <nq/action_sequence.h>
#include <nq/action_spawn.h>
#include <nq/action_repeat.h>
#include <nq/animation.h>
#include "test_main.c"

/* NQ_FE (float equality) is file-local in test_bench.c; redefine here
 * since test_action_reset.c needs it too. */
#define NQ_FE(a, b) ((a) - (b) < 1e-5f && (b) - (a) < 1e-5f)

typedef struct { int target; int ticks; } TickCtx;
static NqActionState counter_tick(NqAction *a, float dt, void *user) {
    (void)a; (void)dt;
    TickCtx *c = (TickCtx *)user;
    if (!c) return NQ_ACTION_FINISHED;
    c->ticks++;
    return c->ticks >= c->target ? NQ_ACTION_FINISHED : NQ_ACTION_RUNNING;
}

static void test_reset_null_safe(void) {
    /* The base reset on a NULL action must be a no-op, not a crash. */
    nq_action_reset(NULL);
    nq_action_set_reset(NULL, NULL);
    NQ_ASSERT(1);
}

/* ----- delay ----- */
static void test_delay_reset(void) {
    NqActionDelay *d = nq_action_delay_create(1.0f);
    NQ_ASSERT_EQ(nq_action_delay_update(d, 0.5f), NQ_ACTION_RUNNING);
    NQ_FE(nq_action_delay_elapsed(d), 0.5f);
    /* Resetting the wrapper's NqAction should reset elapsed to 0 via
     * delay_reset. Subsequent ticks resume from zero. */
    nq_action_reset(nq_action_delay_action(d));
    NQ_FE(nq_action_delay_elapsed(d), 0.0f);
    NQ_ASSERT_EQ(nq_action_state(nq_action_delay_action(d)), NQ_ACTION_RUNNING);
    nq_action_delay_destroy(d);
}

/* ----- tween ----- */
static void test_tween_reset(void) {
    NqAnimFloat anim;
    nq_anim_float_init(&anim, 0.0f, 100.0f, 1.0f, NQ_EASE_LINEAR);
    nq_anim_float_update(&anim, 1.0f);   /* finishes immediately */
    NQ_ASSERT(nq_anim_float_done(&anim));
    nq_action_reset(nq_action_tween_action(nq_action_tween_create(&anim)));
    /* After reset, the anim is back to t=0 and value 0. */
    NQ_FE(nq_anim_float_value(&anim), 0.0f);
}

/* ----- sequence ----- */
static void test_sequence_reset(void) {
    TickCtx c1 = {1, 0}, c2 = {1, 0};
    NqAction *a1 = nq_action_create(counter_tick, NULL, &c1);
    NqAction *a2 = nq_action_create(counter_tick, NULL, &c2);
    const NqAction *subs[] = { a1, a2 };
    NqActionSequence *s = nq_action_sequence_create(subs, 2, 0);
    nq_action_sequence_update(s, 0.016f);  /* a1 finishes */
    nq_action_sequence_update(s, 0.016f);  /* a2 finishes */
    NQ_ASSERT(nq_action_sequence_is_done(s));
    /* Reset the wrapper — current should be back to 0. */
    nq_action_reset(nq_action_sequence_action(s));
    NQ_ASSERT_EQ(nq_action_sequence_current_index(s), 0);
    nq_action_sequence_destroy(s);
    nq_action_destroy(a1);
    nq_action_destroy(a2);
}

/* ----- repeat ----- */
static void test_repeat_finite_reset(void) {
    TickCtx c = {1, 0};
    NqAction *sub = nq_action_create(counter_tick, NULL, &c);
    NqActionRepeat *r = nq_action_repeat_create(sub, 3);
    /* Drive until done. */
    for (int i = 0; i < 20; i++) {
        if (nq_action_state(nq_action_repeat_action(r)) != NQ_ACTION_RUNNING) break;
        nq_action_repeat_update(r, 0.016f);
    }
    NQ_ASSERT(nq_action_repeat_remaining(r) <= 0);
    /* Reset restores the iteration counter to original (3). */
    nq_action_reset(nq_action_repeat_action(r));
    /* remaining is now >= 3 (or less if sub ticked into FINISHED during
     * the next update, but the wrapper is RUNNING again). */
    NQ_ASSERT_EQ(nq_action_state(nq_action_repeat_action(r)), NQ_ACTION_RUNNING);
    NQ_ASSERT(nq_action_repeat_remaining(r) == 3);
    nq_action_repeat_destroy(r);
    nq_action_destroy(sub);
}

static void test_repeat_forever_reset_stays_infinite(void) {
    TickCtx c = {1, 0};
    NqAction *sub = nq_action_create(counter_tick, NULL, &c);
    NqActionRepeat *r = nq_action_repeat_forever_create(sub);
    /* Reset leaves remaining at -1 (forever). */
    nq_action_reset(nq_action_repeat_action(r));
    NQ_ASSERT_EQ(nq_action_repeat_remaining(r), -1);
    nq_action_repeat_destroy(r);
    nq_action_destroy(sub);
}

static void test_set_reset_to_null_disables_reset(void) {
    /* After set_reset(NULL), nq_action_reset is a no-op (state stays FINISHED). */
    NqActionDelay *d = nq_action_delay_create(1.0f);
    nq_action_delay_update(d, 1.0f);  /* finishes */
    NQ_ASSERT(nq_action_is_finished(nq_action_delay_action(d)));
    /* Remove the reset fn. */
    nq_action_set_reset(nq_action_delay_action(d), NULL);
    nq_action_reset(nq_action_delay_action(d));
    /* Without reset_fn, the action's state goes RUNNING (since we flip
     * the flag), but the elapsed counter is NOT zeroed. A subsequent
     * update would tick from elapsed=1.0 (>= duration) and stay FINISHED. */
    NQ_ASSERT_EQ(nq_action_state(nq_action_delay_action(d)), NQ_ACTION_RUNNING);
    nq_action_delay_update(d, 0.016f);
    NQ_ASSERT(nq_action_is_finished(nq_action_delay_action(d)));
    nq_action_delay_destroy(d);
}

NQ_TEST_REGISTER("reset_null_safe",              test_reset_null_safe);
NQ_TEST_REGISTER("reset_delay",                  test_delay_reset);
NQ_TEST_REGISTER("reset_tween",                  test_tween_reset);
NQ_TEST_REGISTER("reset_sequence",               test_sequence_reset);
NQ_TEST_REGISTER("reset_repeat_finite",         test_repeat_finite_reset);
NQ_TEST_REGISTER("reset_repeat_forever",        test_repeat_forever_reset_stays_infinite);
NQ_TEST_REGISTER("reset_set_null_disables",     test_set_reset_to_null_disables_reset);
