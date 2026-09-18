/*
 * nq — tests for extended NqVec2f / NqVec2i math primitives:
 *   dot, cross, length_sq, length, normalize, perp, reflect,
 *   nq_vec2f_eq, nq_vec2i_dot, nq_vec2i_cross.
 */
#include <math.h>
#include <nq/vec.h>
#include "test_main.c"

/* ── helpers ────────────────────────────────────────────────────── */
#define NEAR(a, b) ((a) > (b) - 1e-5f && (a) < (b) + 1e-5f)
#define NQ_ASSERT_NEAR(a, b) \
    NQ_ASSERT(NEAR((float)(a), (float)(b)))

/* ── nq_vec2f_eq ─────────────────────────────────────────────────── */
static void test_vec2f_eq(void) {
    NQ_ASSERT( nq_vec2f_eq(nq_vec2f(1.0f, 2.0f), nq_vec2f(1.0f, 2.0f)));
    NQ_ASSERT(!nq_vec2f_eq(nq_vec2f(1.0f, 2.0f), nq_vec2f(1.0f, 3.0f)));
    NQ_ASSERT(!nq_vec2f_eq(nq_vec2f(0.0f, 0.0f), nq_vec2f(0.0f, 0.1f)));
}

/* ── nq_vec2f_dot ────────────────────────────────────────────────── */
static void test_vec2f_dot_basic(void) {
    /* (1,0)·(0,1) == 0  (perpendicular) */
    NQ_ASSERT_NEAR(nq_vec2f_dot(nq_vec2f(1.0f, 0.0f), nq_vec2f(0.0f, 1.0f)), 0.0f);
    /* (3,4)·(3,4) == 25 */
    NQ_ASSERT_NEAR(nq_vec2f_dot(nq_vec2f(3.0f, 4.0f), nq_vec2f(3.0f, 4.0f)), 25.0f);
    /* (1,2)·(3,4) == 11 */
    NQ_ASSERT_NEAR(nq_vec2f_dot(nq_vec2f(1.0f, 2.0f), nq_vec2f(3.0f, 4.0f)), 11.0f);
}

static void test_vec2f_dot_antiparallel(void) {
    /* Opposite-direction vectors → negative dot. */
    float d = nq_vec2f_dot(nq_vec2f(1.0f, 0.0f), nq_vec2f(-1.0f, 0.0f));
    NQ_ASSERT(d < 0.0f);
    NQ_ASSERT_NEAR(d, -1.0f);
}

/* ── nq_vec2f_cross ──────────────────────────────────────────────── */
static void test_vec2f_cross(void) {
    /* (1,0) × (0,1) == 1  (CCW) */
    NQ_ASSERT_NEAR(nq_vec2f_cross(nq_vec2f(1.0f, 0.0f), nq_vec2f(0.0f, 1.0f)),  1.0f);
    /* (0,1) × (1,0) == -1  (CW) */
    NQ_ASSERT_NEAR(nq_vec2f_cross(nq_vec2f(0.0f, 1.0f), nq_vec2f(1.0f, 0.0f)), -1.0f);
    /* collinear → 0 */
    NQ_ASSERT_NEAR(nq_vec2f_cross(nq_vec2f(2.0f, 4.0f), nq_vec2f(1.0f, 2.0f)),  0.0f);
}

/* ── nq_vec2f_length_sq / nq_vec2f_length ───────────────────────── */
static void test_vec2f_length_sq(void) {
    NQ_ASSERT_NEAR(nq_vec2f_length_sq(nq_vec2f(3.0f, 4.0f)), 25.0f);
    NQ_ASSERT_NEAR(nq_vec2f_length_sq(nq_vec2f(0.0f, 0.0f)),  0.0f);
}

static void test_vec2f_length(void) {
    NQ_ASSERT_NEAR(nq_vec2f_length(nq_vec2f(3.0f, 4.0f)), 5.0f);
    NQ_ASSERT_NEAR(nq_vec2f_length(nq_vec2f(0.0f, 0.0f)), 0.0f);
    NQ_ASSERT_NEAR(nq_vec2f_length(nq_vec2f(1.0f, 0.0f)), 1.0f);
}

/* ── nq_vec2f_normalize ──────────────────────────────────────────── */
static void test_vec2f_normalize_unit(void) {
    /* Already unit — stays unit. */
    NqVec2f n = nq_vec2f_normalize(nq_vec2f(1.0f, 0.0f));
    NQ_ASSERT_NEAR(n.x, 1.0f);
    NQ_ASSERT_NEAR(n.y, 0.0f);
}

static void test_vec2f_normalize_345(void) {
    NqVec2f n = nq_vec2f_normalize(nq_vec2f(3.0f, 4.0f));
    NQ_ASSERT_NEAR(n.x, 0.6f);
    NQ_ASSERT_NEAR(n.y, 0.8f);
    /* Length of result is 1. */
    NQ_ASSERT_NEAR(nq_vec2f_length(n), 1.0f);
}

static void test_vec2f_normalize_zero(void) {
    /* Zero vector → (0, 0), no crash. */
    NqVec2f n = nq_vec2f_normalize(nq_vec2f(0.0f, 0.0f));
    NQ_ASSERT_NEAR(n.x, 0.0f);
    NQ_ASSERT_NEAR(n.y, 0.0f);
}

/* ── nq_vec2f_perp ───────────────────────────────────────────────── */
static void test_vec2f_perp(void) {
    /* (1,0) → (0,1): 90° CCW. */
    NqVec2f p = nq_vec2f_perp(nq_vec2f(1.0f, 0.0f));
    NQ_ASSERT_NEAR(p.x, 0.0f);
    NQ_ASSERT_NEAR(p.y, 1.0f);

    /* (0,1) → (-1,0). */
    NqVec2f q = nq_vec2f_perp(nq_vec2f(0.0f, 1.0f));
    NQ_ASSERT_NEAR(q.x, -1.0f);
    NQ_ASSERT_NEAR(q.y,  0.0f);

    /* Perpendicular of perp is the original (two 90° = 180° → negate;
     * rotating CCW twice gives -v). */
    NqVec2f v  = nq_vec2f(3.0f, 4.0f);
    NqVec2f pp = nq_vec2f_perp(nq_vec2f_perp(v));
    NQ_ASSERT_NEAR(pp.x, -v.x);
    NQ_ASSERT_NEAR(pp.y, -v.y);

    /* perp is perpendicular: dot(v, perp(v)) == 0. */
    NQ_ASSERT_NEAR(nq_vec2f_dot(v, nq_vec2f_perp(v)), 0.0f);
}

/* ── nq_vec2f_reflect ────────────────────────────────────────────── */
static void test_vec2f_reflect_horizontal(void) {
    /* Reflect (1,-1) off a horizontal surface (normal (0,1)) → (1,1). */
    NqVec2f v = nq_vec2f(1.0f, -1.0f);
    NqVec2f n = nq_vec2f(0.0f,  1.0f);
    NqVec2f r = nq_vec2f_reflect(v, n);
    NQ_ASSERT_NEAR(r.x,  1.0f);
    NQ_ASSERT_NEAR(r.y,  1.0f);
}

static void test_vec2f_reflect_vertical(void) {
    /* Reflect (-1,1) off a vertical wall (normal (1,0)) → (1,1). */
    NqVec2f v = nq_vec2f(-1.0f, 1.0f);
    NqVec2f n = nq_vec2f( 1.0f, 0.0f);
    NqVec2f r = nq_vec2f_reflect(v, n);
    NQ_ASSERT_NEAR(r.x,  1.0f);
    NQ_ASSERT_NEAR(r.y,  1.0f);
}

static void test_vec2f_reflect_preserves_length(void) {
    /* Reflection off any unit-normal surface preserves vector length. */
    NqVec2f v = nq_vec2f(3.0f, 4.0f);
    NqVec2f n = nq_vec2f_normalize(nq_vec2f(1.0f, 1.0f));
    NqVec2f r = nq_vec2f_reflect(v, n);
    NQ_ASSERT_NEAR(nq_vec2f_length(r), nq_vec2f_length(v));
}

/* ── nq_vec2i_dot / nq_vec2i_cross ──────────────────────────────── */
static void test_vec2i_dot(void) {
    NQ_ASSERT_EQ(nq_vec2i_dot(nq_vec2i(3, 4), nq_vec2i(3, 4)), 25);
    NQ_ASSERT_EQ(nq_vec2i_dot(nq_vec2i(1, 0), nq_vec2i(0, 1)),  0);
    NQ_ASSERT_EQ(nq_vec2i_dot(nq_vec2i(1, 2), nq_vec2i(3, 4)), 11);
}

static void test_vec2i_cross(void) {
    NQ_ASSERT_EQ(nq_vec2i_cross(nq_vec2i(1, 0), nq_vec2i(0, 1)),  1);
    NQ_ASSERT_EQ(nq_vec2i_cross(nq_vec2i(0, 1), nq_vec2i(1, 0)), -1);
    NQ_ASSERT_EQ(nq_vec2i_cross(nq_vec2i(2, 4), nq_vec2i(1, 2)),  0);
}

/* ── registration ────────────────────────────────────────────────── */
NQ_TEST_REGISTER("vec2f_eq",                    test_vec2f_eq);
NQ_TEST_REGISTER("vec2f_dot_basic",             test_vec2f_dot_basic);
NQ_TEST_REGISTER("vec2f_dot_antiparallel",      test_vec2f_dot_antiparallel);
NQ_TEST_REGISTER("vec2f_cross",                 test_vec2f_cross);
NQ_TEST_REGISTER("vec2f_length_sq",             test_vec2f_length_sq);
NQ_TEST_REGISTER("vec2f_length",                test_vec2f_length);
NQ_TEST_REGISTER("vec2f_normalize_unit",        test_vec2f_normalize_unit);
NQ_TEST_REGISTER("vec2f_normalize_345",         test_vec2f_normalize_345);
NQ_TEST_REGISTER("vec2f_normalize_zero",        test_vec2f_normalize_zero);
NQ_TEST_REGISTER("vec2f_perp",                  test_vec2f_perp);
NQ_TEST_REGISTER("vec2f_reflect_horizontal",    test_vec2f_reflect_horizontal);
NQ_TEST_REGISTER("vec2f_reflect_vertical",      test_vec2f_reflect_vertical);
NQ_TEST_REGISTER("vec2f_reflect_preserves_len", test_vec2f_reflect_preserves_length);
NQ_TEST_REGISTER("vec2i_dot",                   test_vec2i_dot);
NQ_TEST_REGISTER("vec2i_cross",                 test_vec2i_cross);
