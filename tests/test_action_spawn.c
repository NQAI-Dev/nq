#include "nq/action_spawn.h"
#include "test_main.c"

/* "Tick N times then finish" — counter_tick reuses logic from sequence tests. */
typedef struct { int target; int ticks; } TickCtx;

static NqActionState counter_tick(NqAction *a, float dt, void *user) {
    (void)a; (void)dt;
    TickCtx *c = (TickCtx *)user;
    if (!c) return NQ_ACTION_FINISHED;
    c->ticks++;
    return c->ticks >= c->target ? NQ_ACTION_FINISHED : NQ_ACTION_RUNNING;
}

static void test_spawn_create_empty(void) {
    NqActionSpawn *s = nq_action_spawn_create(NULL, 0, 0);
    NQ_ASSERT(s != NULL);
    NQ_ASSERT_EQ(nq_action_spawn_sub_count(s), 0);
    /* Empty spawn: FINISHED on first tick. */
    NQ_ASSERT_EQ(nq_action_spawn_update(s, 0.016f), NQ_ACTION_FINISHED);
    nq_action_spawn_destroy(s);
}

static void test_spawn_ticks_all_subs_per_frame(void) {
    /* 3 subs, each finishes on tick 1. Spawn needs 1 tick total. */
    TickCtx c1 = {1, 0}, c2 = {1, 0}, c3 = {1, 0};
    NqAction *a1 = nq_action_create(counter_tick, NULL, &c1);
    NqAction *a2 = nq_action_create(counter_tick, NULL, &c2);
    NqAction *a3 = nq_action_create(counter_tick, NULL, &c3);
    NqAction *subs[] = { a1, a2, a3 };

    NqActionSpawn *s = nq_action_spawn_create(subs, 3, 0);
    NQ_ASSERT(s != NULL);

    NQ_ASSERT_EQ(nq_action_spawn_update(s, 0.016f), NQ_ACTION_FINISHED);
    NQ_ASSERT_EQ(c1.ticks, 1);
    NQ_ASSERT_EQ(c2.ticks, 1);
    NQ_ASSERT_EQ(c3.ticks, 1);
    NQ_ASSERT_EQ(nq_action_spawn_finished_sub_count(s), 3);
    NQ_ASSERT(nq_action_spawn_is_done(s));

    nq_action_spawn_destroy(s);
    nq_action_destroy(a1);
    nq_action_destroy(a2);
    nq_action_destroy(a3);
}

static void test_spawn_waits_for_slowest_sub(void) {
    /* sub 0 finishes on tick 1, sub 1 on tick 3. Spawn needs 3 ticks total. */
    TickCtx c_fast = {1, 0}, c_slow = {3, 0};
    NqAction *a_fast = nq_action_create(counter_tick, NULL, &c_fast);
    NqAction *a_slow = nq_action_create(counter_tick, NULL, &c_slow);
    NqAction *subs[] = { a_fast, a_slow };

    NqActionSpawn *s = nq_action_spawn_create(subs, 2, 0);
    /* Tick 1: a_fast finishes (ticks 1), a_slow still RUNNING. */
    NQ_ASSERT_EQ(nq_action_spawn_update(s, 0.016f), NQ_ACTION_RUNNING);
    NQ_ASSERT_EQ(nq_action_spawn_finished_sub_count(s), 1);
    /* Tick 2: a_slow ticks 2 (still RUNNING, target=3). */
    NQ_ASSERT_EQ(nq_action_spawn_update(s, 0.016f), NQ_ACTION_RUNNING);
    NQ_ASSERT_EQ(c_slow.ticks, 2);
    /* Tick 3: a_slow reaches target=3, FINISHED. Spawn FINISHED. */
    NQ_ASSERT_EQ(nq_action_spawn_update(s, 0.016f), NQ_ACTION_FINISHED);
    NQ_ASSERT_EQ(nq_action_spawn_finished_sub_count(s), 2);

    nq_action_spawn_destroy(s);
    nq_action_destroy(a_fast);
    nq_action_destroy(a_slow);
}

static void test_spawn_owns_subs(void) {
    TickCtx c = {1, 0};
    NqAction *a = nq_action_create(counter_tick, NULL, &c);
    NqAction *subs[] = { a };
    NqActionSpawn *s = nq_action_spawn_create(subs, 1, 1);  /* take_ownership=1 */
    nq_action_spawn_destroy(s);
    /* If we tried nq_action_destroy(a) here, double-free. Just confirm we
     * got here cleanly. */
    NQ_ASSERT(1);
}

static void test_spawn_add_dynamically(void) {
    NqActionSpawn *s = nq_action_spawn_create(NULL, 0, 0);
    TickCtx c1 = {1, 0}, c2 = {1, 0};
    NqAction *a1 = nq_action_create(counter_tick, NULL, &c1);
    NqAction *a2 = nq_action_create(counter_tick, NULL, &c2);
    NQ_ASSERT_EQ(nq_action_spawn_add(s, a1, 0), 1);
    NQ_ASSERT_EQ(nq_action_spawn_add(s, a2, 0), 1);
    NQ_ASSERT_EQ(nq_action_spawn_sub_count(s), 2);
    /* Empty spawn finishes immediately on construction; re-tick with
     * subs present runs them. */
    (void)nq_action_spawn_update(s, 0.016f);
    NQ_ASSERT_EQ(nq_action_spawn_finished_sub_count(s), 2);
    nq_action_spawn_destroy(s);
    nq_action_destroy(a1);
    nq_action_destroy(a2);
}

static void test_spawn_overflow(void) {
    NqActionSpawn *s = nq_action_spawn_create(NULL, 0, 0);
    TickCtx c = {1, 0};
    NqAction *overflow = NULL;
    for (int i = 0; i < NQ_ACTION_SPAWN_MAX; i++) {
        NqAction *a = nq_action_create(counter_tick, NULL, &c);
        if (nq_action_spawn_add(s, a, 0) != 1) {
            overflow = a;
            break;
        }
    }
    if (!overflow) {
        overflow = nq_action_create(counter_tick, NULL, &c);
    }
    NQ_ASSERT_EQ(nq_action_spawn_add(s, overflow, 0), 0);
    nq_action_spawn_destroy(s);
    /* subs we added are leaked (no public iterator on NqActionSpawn) */
}

static void test_spawn_null_safe(void) {
    nq_action_spawn_destroy(NULL);
    NQ_ASSERT_EQ(nq_action_spawn_update(NULL, 0.016f), NQ_ACTION_FINISHED);
    NQ_ASSERT(nq_action_spawn_action(NULL) == NULL);
    NQ_ASSERT(nq_action_spawn_is_done(NULL));
    NQ_ASSERT_EQ(nq_action_spawn_sub_count(NULL), 0);
    NQ_ASSERT_EQ(nq_action_spawn_finished_sub_count(NULL), 0);
    NQ_ASSERT_EQ(nq_action_spawn_add(NULL, NULL, 0), 0);
}

NQ_TEST_REGISTER("spawn_create_empty",                 test_spawn_create_empty)
NQ_TEST_REGISTER("spawn_ticks_all_per_frame",          test_spawn_ticks_all_subs_per_frame)
NQ_TEST_REGISTER("spawn_waits_for_slowest_sub",       test_spawn_waits_for_slowest_sub)
NQ_TEST_REGISTER("spawn_owns_subs",                    test_spawn_owns_subs)
NQ_TEST_REGISTER("spawn_add_dynamically",              test_spawn_add_dynamically)
NQ_TEST_REGISTER("spawn_overflow",                     test_spawn_overflow)
NQ_TEST_REGISTER("spawn_null_safe",                    test_spawn_null_safe)
