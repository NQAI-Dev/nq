#include <nq/rect.h>
#include "test_main.c"  /* for NQ_TEST_REGISTER / NQ_ASSERT_EQ / NQ_ASSERT */

static void test_rect_create(void) {
    NqRect r = nq_rect(10, 20, 30, 40);
    NQ_ASSERT_EQ(r.x, 10);
    NQ_ASSERT_EQ(r.y, 20);
    NQ_ASSERT_EQ(r.w, 30);
    NQ_ASSERT_EQ(r.h, 40);
}

static void test_rect_empty(void) {
    NQ_ASSERT(nq_rect_empty(nq_rect(0, 0, 0, 10)));
    NQ_ASSERT(nq_rect_empty(nq_rect(0, 0, 10, 0)));
    NQ_ASSERT(nq_rect_empty(nq_rect(0, 0, -5, 5)));
    NQ_ASSERT(!nq_rect_empty(nq_rect(0, 0, 1, 1)));
}

static void test_rect_contains(void) {
    NqRect r = nq_rect(10, 10, 20, 20); /* covers [10,30) x [10,30) */
    /* Inside corners + center */
    NQ_ASSERT(nq_rect_contains(r, nq_vec2i(10, 10))); /* top-left inclusive */
    NQ_ASSERT(nq_rect_contains(r, nq_vec2i(15, 15))); /* center */
    NQ_ASSERT(nq_rect_contains(r, nq_vec2i(29, 29))); /* just inside max edge */
    /* Outside corners */
    NQ_ASSERT(!nq_rect_contains(r, nq_vec2i(9, 15)));  /* left of */
    NQ_ASSERT(!nq_rect_contains(r, nq_vec2i(30, 15))); /* on max edge — half-open */
    NQ_ASSERT(!nq_rect_contains(r, nq_vec2i(15, 30)));
    NQ_ASSERT(!nq_rect_contains(r, nq_vec2i(31, 15)));
}

static void test_rect_intersects(void) {
    NqRect a = nq_rect(0, 0, 10, 10);
    NqRect b = nq_rect(5, 5, 10, 10);
    NqRect c = nq_rect(20, 20, 5, 5);
    NQ_ASSERT(nq_rect_intersects(a, b));
    NQ_ASSERT(nq_rect_intersects(b, a)); /* symmetric */
    NQ_ASSERT(!nq_rect_intersects(a, c));
    NQ_ASSERT(!nq_rect_intersects(c, a));
    /* Sharing an edge: half-open excludes overlap */
    NqRect d = nq_rect(10, 0, 10, 10); /* d.x == a.x + a.w */
    NQ_ASSERT(!nq_rect_intersects(a, d));
    /* Empty rect never intersects */
    NQ_ASSERT(!nq_rect_intersects(a, nq_rect(0, 0, 0, 0)));
}

static void test_rect_intersection(void) {
    NqRect a = nq_rect(0, 0, 10, 10);
    NqRect b = nq_rect(5, 5, 10, 10); /* overlap = [5,10) x [5,10) = 5x5 */
    NqRect i = nq_rect_intersection(a, b);
    NQ_ASSERT_EQ(i.x, 5);
    NQ_ASSERT_EQ(i.y, 5);
    NQ_ASSERT_EQ(i.w, 5);
    NQ_ASSERT_EQ(i.h, 5);
    /* Disjoint -> empty rect */
    NqRect j = nq_rect_intersection(a, nq_rect(20, 20, 5, 5));
    NQ_ASSERT(nq_rect_empty(j));
}

static void test_rect_union(void) {
    NqRect a = nq_rect(0, 0, 5, 5);
    NqRect b = nq_rect(10, 10, 5, 5);
    NqRect u = nq_rect_union(a, b); /* should be [0,15) x [0,15) */
    NQ_ASSERT_EQ(u.x, 0);
    NQ_ASSERT_EQ(u.y, 0);
    NQ_ASSERT_EQ(u.w, 15);
    NQ_ASSERT_EQ(u.h, 15);
    /* Union with empty = other rect */
    NqRect e = nq_rect_union(a, nq_rect(0, 0, 0, 0));
    NQ_ASSERT(nq_rect_eq(e, a));
}

static void test_rect_inflate(void) {
    NqRect r = nq_rect_inflate(nq_rect(10, 10, 20, 20), 5, 3);
    NQ_ASSERT_EQ(r.x, 5);
    NQ_ASSERT_EQ(r.y, 7);
    NQ_ASSERT_EQ(r.w, 30);
    NQ_ASSERT_EQ(r.h, 26);
}

NQ_TEST_REGISTER("rect_create",      test_rect_create);
NQ_TEST_REGISTER("rect_empty",        test_rect_empty);
NQ_TEST_REGISTER("rect_contains",     test_rect_contains);
NQ_TEST_REGISTER("rect_intersects",   test_rect_intersects);
NQ_TEST_REGISTER("rect_intersection", test_rect_intersection);
NQ_TEST_REGISTER("rect_union",        test_rect_union);
NQ_TEST_REGISTER("rect_inflate",      test_rect_inflate);

static void test_rect_center_basic(void) {
    /* 100x200 rect at (10, 20) → center at (60, 120) */
    NqVec2i c = nq_rect_center(nq_rect(10, 20, 100, 200));
    NQ_ASSERT_EQ(c.x, 60);
    NQ_ASSERT_EQ(c.y, 120);
}

static void test_rect_center_origin(void) {
    /* 4x4 rect at origin → center at (2, 2) — integer division */
    NqVec2i c = nq_rect_center(nq_rect(0, 0, 4, 4));
    NQ_ASSERT_EQ(c.x, 2);
    NQ_ASSERT_EQ(c.y, 2);
}

static void test_rect_center_empty(void) {
    /* Empty rect (w=0 or h=0) → returns the top-left corner */
    NQ_ASSERT(nq_vec2i_eq(nq_rect_center(nq_rect(50, 70, 0, 0)), nq_vec2i(50, 70)));
    /* Negative size: still returns top-left (well-defined even when
     * w/h is negative; useful so callers don't need to special-case) */
    NQ_ASSERT(nq_vec2i_eq(nq_rect_center(nq_rect(50, 70, -10, -10)), nq_vec2i(50, 70)));
}

NQ_TEST_REGISTER("rect_center_basic",  test_rect_center_basic);
NQ_TEST_REGISTER("rect_center_origin", test_rect_center_origin);
NQ_TEST_REGISTER("rect_center_empty",  test_rect_center_empty);
