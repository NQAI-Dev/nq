/*
 * nq — unit tests for nq_sweep (sweep-and-prune broadphase).
 *
 * Tests cover: create/destroy invariants, invalid args, clear/reuse,
 * capacity overflow, single object (no pairs), pair detection incl.
 * y-axis pruning, half-open edge convention, unsorted insertion order,
 * degenerate boxes, and a 4-object scene with a known pair set.
 * Determinism check: same insert sequence → same pair sequence.
 */
#include "test_main.c"

#include "nq/sweep.h"

#include <stdint.h>

/* ---------------------------------------------------------------------------
 * Helpers
 * -------------------------------------------------------------------------*/

/* Records every pair callback into flat arrays. */
#define RECV_MAX 64
typedef struct {
    uint32_t a[RECV_MAX];
    uint32_t b[RECV_MAX];
    int      n;
} PairLog;

static void log_pair(uint32_t id_a, uint32_t id_b, void *user) {
    PairLog *log = user;
    if (log->n < RECV_MAX) {
        log->a[log->n] = id_a;
        log->b[log->n] = id_b;
        log->n++;
    }
}

/* Was pair (x, y), in either order, reported? */
static int has_pair(const PairLog *log, uint32_t x, uint32_t y) {
    for (int i = 0; i < log->n; i++) {
        if ((log->a[i] == x && log->b[i] == y) ||
            (log->a[i] == y && log->b[i] == x)) {
            return 1;
        }
    }
    return 0;
}

/* ---------------------------------------------------------------------------
 * Tests
 * -------------------------------------------------------------------------*/

static void test_sweep_create_destroy(void) {
    NqSweep *sw = nq_sweep_create(16);
    NQ_ASSERT(sw != NULL);
    NQ_ASSERT_EQ(nq_sweep_count(sw), 0);
    nq_sweep_destroy(sw);
}

static void test_sweep_invalid_args(void) {
    NQ_ASSERT(nq_sweep_create(0) == NULL); /* zero capacity */
    nq_sweep_destroy(NULL);                /* must not crash */
    nq_sweep_clear(NULL);                  /* must not crash */
    NQ_ASSERT_EQ(nq_sweep_count(NULL), 0);
    NQ_ASSERT_EQ(nq_sweep_for_each_pair(NULL, log_pair, NULL), 0);
    /* Insert into NULL set: dropped, returns 0. */
    NQ_ASSERT_EQ(nq_sweep_insert(NULL, 1, 0, 0, 1, 1), 0);
}

static void test_sweep_empty_no_pairs(void) {
    NqSweep *sw = nq_sweep_create(8);
    NQ_ASSERT(sw != NULL);
    PairLog log = {0};
    NQ_ASSERT_EQ(nq_sweep_for_each_pair(sw, log_pair, &log), 0);
    NQ_ASSERT_EQ(log.n, 0);

    /* Single object can never pair with itself. */
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 7, 0.0f, 0.0f, 10.0f, 10.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_count(sw), 1);
    NQ_ASSERT_EQ(nq_sweep_for_each_pair(sw, log_pair, &log), 0);
    NQ_ASSERT_EQ(log.n, 0);
    nq_sweep_destroy(sw);
}

static void test_sweep_one_pair(void) {
    NqSweep *sw = nq_sweep_create(8);
    NQ_ASSERT(sw != NULL);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 1, 0.0f, 0.0f, 2.0f, 2.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 2, 1.0f, 1.0f, 3.0f, 3.0f), 1);

    PairLog log = {0};
    NQ_ASSERT_EQ(nq_sweep_for_each_pair(sw, log_pair, &log), 1);
    NQ_ASSERT_EQ(log.n, 1);
    NQ_ASSERT(has_pair(&log, 1, 2));
    nq_sweep_destroy(sw);
}

static void test_sweep_y_prune(void) {
    /* x intervals overlap, y intervals do not → no pair. */
    NqSweep *sw = nq_sweep_create(8);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 1, 0.0f, 0.0f, 5.0f, 1.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 2, 1.0f, 5.0f, 6.0f, 6.0f), 1);

    PairLog log = {0};
    NQ_ASSERT_EQ(nq_sweep_for_each_pair(sw, log_pair, &log), 0);
    NQ_ASSERT_EQ(log.n, 0);
    nq_sweep_destroy(sw);
}

static void test_sweep_half_open_edges(void) {
    /* Share only the right edge of the first box: touching ≠ overlapping. */
    NqSweep *sw = nq_sweep_create(8);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 1, 0.0f, 0.0f, 2.0f, 2.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 2, 2.0f, 0.0f, 4.0f, 2.0f), 1);

    PairLog log = {0};
    NQ_ASSERT_EQ(nq_sweep_for_each_pair(sw, log_pair, &log), 0);
    NQ_ASSERT_EQ(log.n, 0);

    /* Same on y: stacked boxes sharing a horizontal edge. */
    nq_sweep_clear(sw);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 1, 0.0f, 0.0f, 2.0f, 2.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 2, 0.0f, 2.0f, 2.0f, 4.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_for_each_pair(sw, log_pair, &log), 0);
    NQ_ASSERT_EQ(log.n, 0);
    nq_sweep_destroy(sw);
}

static void test_sweep_unsorted_insertion(void) {
    /* Insert in reverse x order; the internal sort must fix it. */
    NqSweep *sw = nq_sweep_create(8);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 3, 20.0f, 0.0f, 22.0f, 2.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 2, 10.0f, 0.0f, 12.0f, 2.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 1, 0.0f, 0.0f, 2.0f, 2.0f), 1);

    /* All three x-disjoint: no pairs despite full y overlap. */
    PairLog log = {0};
    NQ_ASSERT_EQ(nq_sweep_for_each_pair(sw, log_pair, &log), 0);
    NQ_ASSERT_EQ(log.n, 0);

    /* Now a big box spanning all three → 3 pairs. */
    nq_sweep_clear(sw);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 3, 20.0f, 0.0f, 22.0f, 2.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 2, 10.0f, 0.0f, 12.0f, 2.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 1, 0.0f, 0.0f, 2.0f, 2.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 9, 0.0f, 1.0f, 100.0f, 1.5f), 1);

    log.n = 0;
    NQ_ASSERT_EQ(nq_sweep_for_each_pair(sw, log_pair, &log), 3);
    NQ_ASSERT(has_pair(&log, 1, 9));
    NQ_ASSERT(has_pair(&log, 2, 9));
    NQ_ASSERT(has_pair(&log, 3, 9));
    NQ_ASSERT(!has_pair(&log, 1, 2));
    NQ_ASSERT(!has_pair(&log, 1, 3));
    NQ_ASSERT(!has_pair(&log, 2, 3));
    nq_sweep_destroy(sw);
}

static void test_sweep_negative_coords(void) {
    NqSweep *sw = nq_sweep_create(8);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 1, -10.0f, -10.0f, -8.0f, -8.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 2, -9.5f, -9.5f, -7.0f, -7.0f), 1);

    PairLog log = {0};
    NQ_ASSERT_EQ(nq_sweep_for_each_pair(sw, log_pair, &log), 1);
    NQ_ASSERT(has_pair(&log, 1, 2));
    nq_sweep_destroy(sw);
}

static void test_sweep_degenerate_boxes(void) {
    /* Zero-area boxes never pair (max == min fails strict overlap). */
    NqSweep *sw = nq_sweep_create(8);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 1, 0.0f, 0.0f, 0.0f, 0.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 2, 0.0f, 0.0f, 5.0f, 5.0f), 1);

    PairLog log = {0};
    NQ_ASSERT_EQ(nq_sweep_for_each_pair(sw, log_pair, &log), 0);
    NQ_ASSERT_EQ(log.n, 0);
    nq_sweep_destroy(sw);
}

static void test_sweep_overflow_and_clear(void) {
    NqSweep *sw = nq_sweep_create(2);
    NQ_ASSERT(sw != NULL);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 1, 0.0f, 0.0f, 1.0f, 1.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 2, 0.5f, 0.0f, 1.5f, 1.0f), 1);
    /* Full: dropped. */
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 3, 0.0f, 0.0f, 1.0f, 1.0f), 0);
    NQ_ASSERT_EQ(nq_sweep_count(sw), 2);

    PairLog log = {0};
    NQ_ASSERT_EQ(nq_sweep_for_each_pair(sw, log_pair, &log), 1);

    /* Clear resets capacity; count back to 0. */
    nq_sweep_clear(sw);
    NQ_ASSERT_EQ(nq_sweep_count(sw), 0);
    log.n = 0;
    NQ_ASSERT_EQ(nq_sweep_for_each_pair(sw, log_pair, &log), 0);

    /* Reusable after clear. */
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 4, 0.0f, 0.0f, 1.0f, 1.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 5, 0.5f, 0.0f, 1.5f, 1.0f), 1);
    log.n = 0;
    NQ_ASSERT_EQ(nq_sweep_for_each_pair(sw, log_pair, &log), 1);
    NQ_ASSERT(has_pair(&log, 4, 5));
    nq_sweep_destroy(sw);
}

static void test_sweep_null_callback_counts(void) {
    /* fn == NULL: pure pair counter. */
    NqSweep *sw = nq_sweep_create(8);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 1, 0.0f, 0.0f, 3.0f, 3.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 2, 1.0f, 1.0f, 4.0f, 4.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 3, 2.0f, 2.0f, 5.0f, 5.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_for_each_pair(sw, NULL, NULL), 3);
    nq_sweep_destroy(sw);
}

static void test_sweep_scene_known_pairs(void) {
    /*
     *  12 ┌────────┐  id=3
     *     │        │
     *   6 └──┐     │
     *  11 ┌──┴──┐  │   ┌───────┐
     *     │ id=1 │  │   │ id=3   │
     *   3 └─────┘  │ ┌─┴───────┐
     *   0          │ │ id=2     │
     *              └─┴─────────┘
     *      0   5  8  16        20
     *
     * Expected pairs: 1-2 (overlap), 2-3 (overlap), 1-3 NOT (disjoint).
     */
    NqSweep *sw = nq_sweep_create(4);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 3, 8.0f, 5.0f, 16.0f, 12.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 1, 0.0f, 3.0f, 7.0f, 11.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 2, 6.0f, 0.0f, 18.0f, 6.0f), 1);

    PairLog log = {0};
    NQ_ASSERT_EQ(nq_sweep_for_each_pair(sw, log_pair, &log), 2);
    NQ_ASSERT_EQ(log.n, 2);
    NQ_ASSERT(has_pair(&log, 1, 2));
    NQ_ASSERT(has_pair(&log, 2, 3));
    NQ_ASSERT(!has_pair(&log, 1, 3));
    nq_sweep_destroy(sw);
}

static void test_sweep_deterministic_repeat(void) {
    /* Same data, two sweeps → same pair sequences (sort is stable). */
    NqSweep *sw = nq_sweep_create(4);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 1, 0.0f, 0.0f, 5.0f, 5.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 2, 1.0f, 1.0f, 6.0f, 6.0f), 1);
    NQ_ASSERT_EQ(nq_sweep_insert(sw, 3, 2.0f, 2.0f, 7.0f, 7.0f), 1);

    PairLog first = {0};
    PairLog second = {0};
    NQ_ASSERT_EQ(nq_sweep_for_each_pair(sw, log_pair, &first), 3);
    NQ_ASSERT_EQ(nq_sweep_for_each_pair(sw, log_pair, &second), 3);

    NQ_ASSERT_EQ(first.n, second.n);
    for (int i = 0; i < first.n; i++) {
        NQ_ASSERT_EQ(first.a[i], second.a[i]);
        NQ_ASSERT_EQ(first.b[i], second.b[i]);
    }
    nq_sweep_destroy(sw);
}

NQ_TEST_REGISTER("sweep_create_destroy", test_sweep_create_destroy)
NQ_TEST_REGISTER("sweep_invalid_args", test_sweep_invalid_args)
NQ_TEST_REGISTER("sweep_empty_no_pairs", test_sweep_empty_no_pairs)
NQ_TEST_REGISTER("sweep_one_pair", test_sweep_one_pair)
NQ_TEST_REGISTER("sweep_y_prune", test_sweep_y_prune)
NQ_TEST_REGISTER("sweep_half_open_edges", test_sweep_half_open_edges)
NQ_TEST_REGISTER("sweep_unsorted_insertion", test_sweep_unsorted_insertion)
NQ_TEST_REGISTER("sweep_negative_coords", test_sweep_negative_coords)
NQ_TEST_REGISTER("sweep_degenerate_boxes", test_sweep_degenerate_boxes)
NQ_TEST_REGISTER("sweep_overflow_and_clear", test_sweep_overflow_and_clear)
NQ_TEST_REGISTER("sweep_null_callback_counts", test_sweep_null_callback_counts)
NQ_TEST_REGISTER("sweep_scene_known_pairs", test_sweep_scene_known_pairs)
NQ_TEST_REGISTER("sweep_deterministic_repeat", test_sweep_deterministic_repeat)
