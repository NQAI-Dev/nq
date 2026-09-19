/*
 * Tests for nq/rand.h — PCG32 deterministic RNG.
 */
#include <nq/rand.h>
#include "test_main.c"

/* ── determinism ─────────────────────────────────────────────────────── */

static void test_rand_determinism(void) {
    NqRand a, b;
    nq_rand_seed(&a, 42u, 1u);
    nq_rand_seed(&b, 42u, 1u);
    for (int i = 0; i < 1000; i++) {
        NQ_ASSERT(nq_rand_u32(&a) == nq_rand_u32(&b));
    }
}

static void test_rand_different_seeds(void) {
    NqRand a, b;
    nq_rand_seed(&a, 1u, 1u);
    nq_rand_seed(&b, 2u, 1u);
    /* With overwhelmingly high probability the first value differs. */
    uint32_t va = nq_rand_u32(&a);
    uint32_t vb = nq_rand_u32(&b);
    NQ_ASSERT(va != vb);
}

static void test_rand_different_seq(void) {
    NqRand a, b;
    nq_rand_seed(&a, 42u, 1u);
    nq_rand_seed(&b, 42u, 2u);
    uint32_t va = nq_rand_u32(&a);
    uint32_t vb = nq_rand_u32(&b);
    NQ_ASSERT(va != vb);
}

/* ── known value (PCG32 reference) ──────────────────────────────────── */

static void test_rand_known_value(void) {
    /* Reference output from the canonical PCG32 implementation with
     * seed=42, seq=54 (the example from pcg-random.org). */
    NqRand rng;
    nq_rand_seed(&rng, 42u, 54u);
    uint32_t v = nq_rand_u32(&rng);
    /* This exact value matches the pcg32_srandom(42,54) + pcg32_random()
     * output from the reference C implementation. */
    NQ_ASSERT(v == 0xa15c02b7u);
}

/* ── float range ─────────────────────────────────────────────────────── */

static void test_rand_f_range(void) {
    NqRand rng;
    nq_rand_seed(&rng, 123u, 1u);
    int below_zero = 0, at_or_above_one = 0;
    for (int i = 0; i < 100000; i++) {
        float v = nq_rand_f(&rng);
        if (v < 0.0f) below_zero++;
        if (v >= 1.0f) at_or_above_one++;
    }
    NQ_ASSERT(below_zero == 0);
    NQ_ASSERT(at_or_above_one == 0);
}

/* ── integer range ───────────────────────────────────────────────────── */

static void test_rand_range_bounds(void) {
    NqRand rng;
    nq_rand_seed(&rng, 7u, 3u);
    for (int i = 0; i < 10000; i++) {
        int v = nq_rand_range(&rng, -5, 5);
        NQ_ASSERT(v >= -5);
        NQ_ASSERT(v <= 5);
    }
}

static void test_rand_range_single(void) {
    /* lo == hi must always return lo. */
    NqRand rng;
    nq_rand_seed(&rng, 0u, 1u);
    for (int i = 0; i < 100; i++) {
        NQ_ASSERT(nq_rand_range(&rng, 42, 42) == 42);
    }
}

static void test_rand_range_coverage(void) {
    /* All values in a small range must appear within a reasonable budget. */
    NqRand rng;
    nq_rand_seed(&rng, 99u, 1u);
    int seen[10] = {0};
    for (int i = 0; i < 10000; i++) {
        int v = nq_rand_range(&rng, 0, 9);
        seen[v]++;
    }
    for (int k = 0; k < 10; k++) {
        NQ_ASSERT(seen[k] > 0);
    }
}

/* ── float range helper ──────────────────────────────────────────────── */

static void test_rand_range_f_bounds(void) {
    NqRand rng;
    nq_rand_seed(&rng, 55u, 2u);
    for (int i = 0; i < 50000; i++) {
        float v = nq_rand_range_f(&rng, -10.0f, 10.0f);
        NQ_ASSERT(v >= -10.0f);
        NQ_ASSERT(v < 10.0f);
    }
}

/* ── shuffle ─────────────────────────────────────────────────────────── */

static void test_rand_shuffle_permutation(void) {
    /* After shuffle every value still appears exactly once. */
    NqRand rng;
    nq_rand_seed(&rng, 11u, 1u);
    int arr[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    nq_rand_shuffle(&rng, arr, 8, (int)sizeof(int));
    int seen[8] = {0};
    for (int i = 0; i < 8; i++) {
        NQ_ASSERT(arr[i] >= 0 && arr[i] < 8);
        seen[arr[i]]++;
    }
    for (int k = 0; k < 8; k++) {
        NQ_ASSERT(seen[k] == 1);
    }
}

static void test_rand_shuffle_single(void) {
    /* Single-element array stays unchanged. */
    NqRand rng;
    nq_rand_seed(&rng, 1u, 1u);
    int arr[1] = {42};
    nq_rand_shuffle(&rng, arr, 1, (int)sizeof(int));
    NQ_ASSERT(arr[0] == 42);
}

static void test_rand_shuffle_determinism(void) {
    NqRand a, b;
    nq_rand_seed(&a, 77u, 1u);
    nq_rand_seed(&b, 77u, 1u);
    int arr_a[6] = {1, 2, 3, 4, 5, 6};
    int arr_b[6] = {1, 2, 3, 4, 5, 6};
    nq_rand_shuffle(&a, arr_a, 6, (int)sizeof(int));
    nq_rand_shuffle(&b, arr_b, 6, (int)sizeof(int));
    for (int i = 0; i < 6; i++) {
        NQ_ASSERT(arr_a[i] == arr_b[i]);
    }
}

/* ── registration ────────────────────────────────────────────────────── */

NQ_TEST_REGISTER("rand_determinism",        test_rand_determinism)
NQ_TEST_REGISTER("rand_different_seeds",    test_rand_different_seeds)
NQ_TEST_REGISTER("rand_different_seq",      test_rand_different_seq)
NQ_TEST_REGISTER("rand_known_value",        test_rand_known_value)
NQ_TEST_REGISTER("rand_f_range",            test_rand_f_range)
NQ_TEST_REGISTER("rand_range_bounds",       test_rand_range_bounds)
NQ_TEST_REGISTER("rand_range_single",       test_rand_range_single)
NQ_TEST_REGISTER("rand_range_coverage",     test_rand_range_coverage)
NQ_TEST_REGISTER("rand_range_f_bounds",     test_rand_range_f_bounds)
NQ_TEST_REGISTER("rand_shuffle_permutation",test_rand_shuffle_permutation)
NQ_TEST_REGISTER("rand_shuffle_single",     test_rand_shuffle_single)
NQ_TEST_REGISTER("rand_shuffle_determinism",test_rand_shuffle_determinism)
