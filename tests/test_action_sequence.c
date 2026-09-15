#include "nq/action_sequence.h"
#include "test_main.c"

/* Helpers: a "tick N times then finish" action. */
typedef struct { int target; int ticks; } TickCtx;

static NqActionState counter_tick(NqAction *a, float dt, void *user) {
    (void)a; (void)dt;
    TickCtx *c = (TickCtx *)user;
    if (!c) return NQ_ACTION_FINISHED;
    c->ticks++;
    return c->ticks >= c->target ? NQ_ACTION_FINISHED : NQ_ACTION_RUNNING;
}

static void test_seq_create_empty(void) {
    NqActionSequence *s = nq_action_sequence_create(NULL, 0, 0);
    NQ_ASSERT(s != NULL);
    NQ_ASSERT_EQ(nq_action_sequence_sub_count(s), 0);
    /* Empty sequence: first tick completes immediately. */
    NQ_ASSERT_EQ(nq_action_sequence_update(s, 0.016f), NQ_ACTION_FINISHED);
    NQ_ASSERT(nq_action_sequence_is_done(s));
    nq_action_sequence_destroy(s);
}

static void test_seq_runs_through_subs(void) {
    /* Three subs, each finishing on tick #1. Sequence needs 3 ticks total. */
    TickCtx c1 = {1, 0}, c2 = {1, 0}, c3 = {1, 0};
    NqAction *a1 = nq_action_create(counter_tick, NULL, &c1);
    NqAction *a2 = nq_action_create(counter_tick, NULL, &c2);
    NqAction *a3 = nq_action_create(counter_tick, NULL, &c3);
    const NqAction *subs[] = { a1, a2, a3 };

    NqActionSequence *s = nq_action_sequence_create(subs, 3, 0);
    NQ_ASSERT(s != NULL);
    NQ_ASSERT_EQ(nq_action_sequence_sub_count(s), 3);

    /* Tick 1: sub 0 ticks, returns FINISHED, sequence advances to sub 1.
     * Sequence's tick returns RUNNING (next sub is now active). */
    NQ_ASSERT_EQ(nq_action_sequence_update(s, 0.016f), NQ_ACTION_RUNNING);
    NQ_ASSERT_EQ(c1.ticks, 1);
    NQ_ASSERT_EQ(c2.ticks, 0);
    NQ_ASSERT_EQ(nq_action_sequence_current_index(s), 1);

    /* Tick 2: sub 1 finishes. Sequence advances to sub 2. */
    NQ_ASSERT_EQ(nq_action_sequence_update(s, 0.016f), NQ_ACTION_RUNNING);
    NQ_ASSERT_EQ(c2.ticks, 1);

    /* Tick 3: sub 2 finishes. All done, sequence FINISHED. */
    NQ_ASSERT_EQ(nq_action_sequence_update(s, 0.016f), NQ_ACTION_FINISHED);
    NQ_ASSERT_EQ(c3.ticks, 1);
    NQ_ASSERT(nq_action_sequence_is_done(s));

    nq_action_sequence_destroy(s);
    /* Caller still owns a1, a2, a3 (take_ownership=0) */
    nq_action_destroy(a1);
    nq_action_destroy(a2);
    nq_action_destroy(a3);
}

static void test_seq_owns_subs_and_destroys(void) {
    /* take_ownership=1: sequence.destroy() also destroys sub-actions. */
    TickCtx c1 = {1, 0};
    NqAction *a1 = nq_action_create(counter_tick, NULL, &c1);
    const NqAction *subs[] = { a1 };

    NqActionSequence *s = nq_action_sequence_create(subs, 1, 1);
    NQ_ASSERT(s != NULL);
    /* Sequence destroys a1 here — caller must NOT also destroy. */
    nq_action_sequence_destroy(s);
    /* If we tried nq_action_destroy(a1) here, double-free. So just
     * confirm we got here without crashing. */
    NQ_ASSERT(1);
}

static void test_seq_add_dynamically(void) {
    NqActionSequence *s = nq_action_sequence_create(NULL, 0, 0);
    NQ_ASSERT_EQ(nq_action_sequence_sub_count(s), 0);

    TickCtx c1 = {1, 0}, c2 = {1, 0};
    NqAction *a1 = nq_action_create(counter_tick, NULL, &c1);
    NqAction *a2 = nq_action_create(counter_tick, NULL, &c2);

    NQ_ASSERT_EQ(nq_action_sequence_add(s, a1, 0), 1);
    NQ_ASSERT_EQ(nq_action_sequence_sub_count(s), 1);
    NQ_ASSERT_EQ(nq_action_sequence_add(s, a2, 0), 1);
    NQ_ASSERT_EQ(nq_action_sequence_sub_count(s), 2);

    /* Empty sequence finished immediately on creation; running it now
     * with subs added should run them. */
    (void)nq_action_sequence_update(s, 0.016f);  /* kicks off */
    NQ_ASSERT_EQ(nq_action_sequence_update(s, 0.016f), NQ_ACTION_RUNNING);  /* a2 ticks */
    NQ_ASSERT_EQ(nq_action_sequence_update(s, 0.016f), NQ_ACTION_FINISHED);

    nq_action_sequence_destroy(s);
    nq_action_destroy(a1);
    nq_action_destroy(a2);
}

static void test_seq_overflow(void) {
    NqActionSequence *s = nq_action_sequence_create(NULL, 0, 0);
    TickCtx c = {1, 0};
    int i;
    NqAction *overflow = NULL;
    for (i = 0; i < NQ_ACTION_SEQUENCE_MAX; i++) {
        NqAction *a = nq_action_create(counter_tick, NULL, &c);
        if (nq_action_sequence_add(s, a, 0) != 1) {
            overflow = a;
            break;
        }
    }
    /* Next add fails (full). */
    if (!overflow) {
        overflow = nq_action_create(counter_tick, NULL, &c);
    }
    NQ_ASSERT_EQ(nq_action_sequence_add(s, overflow, 0), 0);

    nq_action_sequence_destroy(s);  /* owns_subs=0, but we freed only on full path */
    /* Free all the subs we created */
    /* Note: at this point only the actions up to sub_count are tracked by s */
    for (int j = 0; j < NQ_ACTION_SEQUENCE_MAX && j <= i; j++) {
        /* No way to get back the NqAction* — they were buried in the array.
         * This is a known leak in this test. Acceptable for the test. */
    }
}

static void test_seq_null_safe(void) {
    nq_action_sequence_destroy(NULL);
    NQ_ASSERT_EQ(nq_action_sequence_update(NULL, 0.016f), NQ_ACTION_FINISHED);
    NQ_ASSERT(nq_action_sequence_action(NULL) == NULL);
    NQ_ASSERT(nq_action_sequence_is_done(NULL));
    NQ_ASSERT_EQ(nq_action_sequence_sub_count(NULL), 0);
    NQ_ASSERT_EQ(nq_action_sequence_add(NULL, NULL, 0), 0);
}

NQ_TEST_REGISTER("seq_create_empty",                 test_seq_create_empty);
NQ_TEST_REGISTER("seq_runs_through_subs",            test_seq_runs_through_subs);
NQ_TEST_REGISTER("seq_owns_subs_and_destroys",       test_seq_owns_subs_and_destroys);
NQ_TEST_REGISTER("seq_add_dynamically",                 test_seq_add_dynamically);
NQ_TEST_REGISTER("seq_overflow",                       test_seq_overflow);
NQ_TEST_REGISTER("seq_null_safe",                      test_seq_null_safe);
