/*
 * nq — unit tests for nq_spatial_hash.
 *
 * Tests cover: basic insert + query, multi-cell AABBs, deduplication,
 * empty-rect handling, negative-coordinate cells, pool overflow (silent
 * drop), clear/reuse, and the entry_count diagnostic.
 */
#include "test_main.c"

#include "nq/spatial_hash.h"
#include "nq/rect.h"

#include <stdint.h>

/* ---------------------------------------------------------------------------
 * Helpers
 * -------------------------------------------------------------------------*/

/* Collect all candidates from a query into an array; return count. */
static int collect_query(NqSpatialHash *sh, NqRect aabb,
                         uint32_t *out, int out_max) {
    NqSpatialHashQuery q = nq_spatial_hash_query_begin(sh, aabb);
    int n = 0;
    uint32_t id;
    while (n < out_max && nq_spatial_hash_query_next(&q, &id)) {
        out[n++] = id;
    }
    return n;
}

static int contains_id(const uint32_t *arr, int n, uint32_t id) {
    for (int i = 0; i < n; i++) {
        if (arr[i] == id) return 1;
    }
    return 0;
}

/* ---------------------------------------------------------------------------
 * Tests
 * -------------------------------------------------------------------------*/

static void test_spatial_hash_create_destroy(void) {
    NqSpatialHash *sh = nq_spatial_hash_create(32, 64, 512);
    NQ_ASSERT(sh != NULL);
    NQ_ASSERT(nq_spatial_hash_entry_count(sh) == 0);
    nq_spatial_hash_destroy(sh);
}

static void test_spatial_hash_invalid_args(void) {
    NQ_ASSERT(nq_spatial_hash_create(0,  64, 512) == NULL); /* zero cell size */
    NQ_ASSERT(nq_spatial_hash_create(32,  0, 512) == NULL); /* zero buckets   */
    NQ_ASSERT(nq_spatial_hash_create(32, 64,   0) == NULL); /* zero entries   */
    nq_spatial_hash_destroy(NULL); /* must not crash */
}

static void test_spatial_hash_single_insert_query(void) {
    NqSpatialHash *sh = nq_spatial_hash_create(32, 64, 256);
    NQ_ASSERT(sh != NULL);

    /* Insert object 1 at a small rect that fits in one cell. */
    NqRect r = nq_rect(10, 10, 5, 5); /* cell (0,0) for cell_size=32 */
    int cells = nq_spatial_hash_insert(sh, 1, r);
    NQ_ASSERT(cells == 1);
    NQ_ASSERT(nq_spatial_hash_entry_count(sh) == 1);

    /* Query the same region — must find object 1. */
    uint32_t out[8];
    int n = collect_query(sh, r, out, 8);
    NQ_ASSERT(n == 1);
    NQ_ASSERT(out[0] == 1);

    /* Query a non-overlapping region — must find nothing. */
    NqRect miss = nq_rect(200, 200, 10, 10);
    n = collect_query(sh, miss, out, 8);
    NQ_ASSERT(n == 0);

    nq_spatial_hash_destroy(sh);
}

static void test_spatial_hash_multi_cell_aabb(void) {
    /* cell_size=32; insert a 70x70 AABB starting at (5,5):
     * x range [5,74] covers cells 0,1,2; y likewise → 9 cells total,
     * 1 unique object. */
    NqSpatialHash *sh = nq_spatial_hash_create(32, 64, 256);
    NQ_ASSERT(sh != NULL);

    NqRect big = nq_rect(5, 5, 70, 70);
    int cells = nq_spatial_hash_insert(sh, 42, big);
    NQ_ASSERT_EQ(cells, 9);
    NQ_ASSERT_EQ((int)nq_spatial_hash_entry_count(sh), 9);

    /* Query should return exactly one unique id (42). */
    uint32_t out[16];
    int n = collect_query(sh, big, out, 16);
    NQ_ASSERT(n == 1);
    NQ_ASSERT(out[0] == 42);

    nq_spatial_hash_destroy(sh);
}

static void test_spatial_hash_two_objects_overlap(void) {
    NqSpatialHash *sh = nq_spatial_hash_create(32, 64, 256);
    NQ_ASSERT(sh != NULL);

    /* Both objects share cell (0,0). */
    nq_spatial_hash_insert(sh, 10, nq_rect(0, 0, 10, 10));
    nq_spatial_hash_insert(sh, 20, nq_rect(5, 5, 10, 10));

    uint32_t out[16];
    int n = collect_query(sh, nq_rect(0, 0, 20, 20), out, 16);
    NQ_ASSERT(n == 2);
    NQ_ASSERT(contains_id(out, n, 10));
    NQ_ASSERT(contains_id(out, n, 20));

    nq_spatial_hash_destroy(sh);
}

static void test_spatial_hash_two_objects_separate_cells(void) {
    NqSpatialHash *sh = nq_spatial_hash_create(32, 64, 256);
    NQ_ASSERT(sh != NULL);

    /* Object A lives in cell (0,0); object B lives in cell (3,3). */
    nq_spatial_hash_insert(sh, 1, nq_rect(0,   0,  10, 10)); /* cell (0,0) */
    nq_spatial_hash_insert(sh, 2, nq_rect(100, 100, 10, 10)); /* cell (3,3) */

    uint32_t out[8];

    /* Query cell (0,0) region — only object 1. */
    int n = collect_query(sh, nq_rect(0, 0, 10, 10), out, 8);
    NQ_ASSERT(n == 1);
    NQ_ASSERT(out[0] == 1);

    /* Query cell (3,3) region — only object 2. */
    n = collect_query(sh, nq_rect(100, 100, 10, 10), out, 8);
    NQ_ASSERT(n == 1);
    NQ_ASSERT(out[0] == 2);

    nq_spatial_hash_destroy(sh);
}

static void test_spatial_hash_empty_rect_ignored(void) {
    NqSpatialHash *sh = nq_spatial_hash_create(32, 64, 256);
    NQ_ASSERT(sh != NULL);

    int cells = nq_spatial_hash_insert(sh, 99, nq_rect(0, 0,  0, 10)); /* w=0 */
    NQ_ASSERT(cells == 0);
    cells = nq_spatial_hash_insert(sh, 99, nq_rect(0, 0, 10,  0)); /* h=0 */
    NQ_ASSERT(cells == 0);
    cells = nq_spatial_hash_insert(sh, 99, nq_rect(0, 0, -1, -1)); /* negative */
    NQ_ASSERT(cells == 0);

    NQ_ASSERT(nq_spatial_hash_entry_count(sh) == 0);

    /* Query on empty rect must return nothing and not crash. */
    uint32_t out[4];
    int n = collect_query(sh, nq_rect(0, 0, 0, 0), out, 4);
    NQ_ASSERT(n == 0);

    nq_spatial_hash_destroy(sh);
}

static void test_spatial_hash_negative_coordinates(void) {
    NqSpatialHash *sh = nq_spatial_hash_create(32, 64, 256);
    NQ_ASSERT(sh != NULL);

    /* Object A at (-40, -40): cell (-2,-2) for cell_size=32. */
    nq_spatial_hash_insert(sh, 7, nq_rect(-40, -40, 10, 10));

    uint32_t out[8];
    int n = collect_query(sh, nq_rect(-40, -40, 10, 10), out, 8);
    NQ_ASSERT(n == 1);
    NQ_ASSERT(out[0] == 7);

    /* Positive region must not find it. */
    n = collect_query(sh, nq_rect(0, 0, 10, 10), out, 8);
    NQ_ASSERT(n == 0);

    nq_spatial_hash_destroy(sh);
}

static void test_spatial_hash_clear_reuse(void) {
    NqSpatialHash *sh = nq_spatial_hash_create(32, 64, 256);
    NQ_ASSERT(sh != NULL);

    nq_spatial_hash_insert(sh, 5, nq_rect(0, 0, 10, 10));
    NQ_ASSERT(nq_spatial_hash_entry_count(sh) == 1);

    nq_spatial_hash_clear(sh);
    NQ_ASSERT(nq_spatial_hash_entry_count(sh) == 0);

    /* After clear the old entry must not be queryable. */
    uint32_t out[4];
    int n = collect_query(sh, nq_rect(0, 0, 10, 10), out, 4);
    NQ_ASSERT(n == 0);

    /* Re-insert after clear must work. */
    nq_spatial_hash_insert(sh, 99, nq_rect(0, 0, 10, 10));
    n = collect_query(sh, nq_rect(0, 0, 10, 10), out, 4);
    NQ_ASSERT(n == 1);
    NQ_ASSERT(out[0] == 99);

    nq_spatial_hash_destroy(sh);
}

static void test_spatial_hash_pool_overflow_silent(void) {
    /* max_entries=2, insert 3 single-cell objects — third is silently dropped. */
    NqSpatialHash *sh = nq_spatial_hash_create(32, 64, 2);
    NQ_ASSERT(sh != NULL);

    int c1 = nq_spatial_hash_insert(sh, 1, nq_rect(0,  0,  10, 10));
    int c2 = nq_spatial_hash_insert(sh, 2, nq_rect(10, 10, 10, 10));
    int c3 = nq_spatial_hash_insert(sh, 3, nq_rect(20, 20, 10, 10)); /* dropped */
    NQ_ASSERT(c1 == 1);
    NQ_ASSERT(c2 == 1);
    NQ_ASSERT(c3 == 0); /* pool exhausted — returns 0 */
    NQ_ASSERT(nq_spatial_hash_entry_count(sh) == 2);

    nq_spatial_hash_destroy(sh);
}

static void test_spatial_hash_deduplication(void) {
    /* A large AABB spans 4 cells; querying with the same AABB must return
     * the object only once, not four times. */
    NqSpatialHash *sh = nq_spatial_hash_create(32, 64, 256);
    NQ_ASSERT(sh != NULL);

    NqRect big = nq_rect(5, 5, 70, 70); /* spans 4 cells */
    nq_spatial_hash_insert(sh, 11, big);

    uint32_t out[16];
    int n = collect_query(sh, big, out, 16);
    NQ_ASSERT(n == 1); /* deduplicated */
    NQ_ASSERT(out[0] == 11);

    nq_spatial_hash_destroy(sh);
}

static void test_spatial_hash_many_objects(void) {
    /* Insert 16 objects in a 4×4 grid of cells and query the full region. */
    NqSpatialHash *sh = nq_spatial_hash_create(32, 128, 4096);
    NQ_ASSERT(sh != NULL);

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            uint32_t id = (uint32_t)(y * 4 + x + 1); /* 1..16 */
            nq_spatial_hash_insert(sh, id, nq_rect(x * 32, y * 32, 10, 10));
        }
    }

    /* Query the entire 4×4 area. */
    uint32_t out[32];
    int n = collect_query(sh, nq_rect(0, 0, 128, 128), out, 32);
    NQ_ASSERT(n == 16);
    for (uint32_t id = 1; id <= 16; id++) {
        NQ_ASSERT(contains_id(out, n, id));
    }

    nq_spatial_hash_destroy(sh);
}

static void test_spatial_hash_query_null_sh(void) {
    /* query_begin with NULL sh must not crash and must yield nothing. */
    NqSpatialHashQuery q = nq_spatial_hash_query_begin(NULL, nq_rect(0, 0, 10, 10));
    uint32_t id;
    int r = nq_spatial_hash_query_next(&q, &id);
    NQ_ASSERT(r == 0);
}

/* Registration */
NQ_TEST_REGISTER("spatial_hash_create_destroy",     test_spatial_hash_create_destroy)
NQ_TEST_REGISTER("spatial_hash_invalid_args",       test_spatial_hash_invalid_args)
NQ_TEST_REGISTER("spatial_hash_single_insert_query",test_spatial_hash_single_insert_query)
NQ_TEST_REGISTER("spatial_hash_multi_cell_aabb",    test_spatial_hash_multi_cell_aabb)
NQ_TEST_REGISTER("spatial_hash_two_objects_overlap",test_spatial_hash_two_objects_overlap)
NQ_TEST_REGISTER("spatial_hash_two_objects_separate_cells",
                                                    test_spatial_hash_two_objects_separate_cells)
NQ_TEST_REGISTER("spatial_hash_empty_rect_ignored", test_spatial_hash_empty_rect_ignored)
NQ_TEST_REGISTER("spatial_hash_negative_coordinates",
                                                    test_spatial_hash_negative_coordinates)
NQ_TEST_REGISTER("spatial_hash_clear_reuse",        test_spatial_hash_clear_reuse)
NQ_TEST_REGISTER("spatial_hash_pool_overflow_silent",
                                                    test_spatial_hash_pool_overflow_silent)
NQ_TEST_REGISTER("spatial_hash_deduplication",      test_spatial_hash_deduplication)
NQ_TEST_REGISTER("spatial_hash_many_objects",       test_spatial_hash_many_objects)
NQ_TEST_REGISTER("spatial_hash_query_null_sh",      test_spatial_hash_query_null_sh)
