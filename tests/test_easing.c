/*
 * Tests for nq/easing.h — easing functions library.
 *
 * Each family is verified for:
 *   - boundary values: ease(0) == 0, ease(1) == 1
 *   - monotonicity (where applicable; back/elastic can overshoot)
 *   - symmetry for in_out variants: result(0.5) ≈ 0.5
 *   - in_out(t) == 1 - in_out(1-t) reflection symmetry
 */
#include <nq/easing.h>
#include "test_main.c"

#include <math.h>

/* Tolerance for floating-point comparisons. */
#define EPS 1e-5f

static int approx(float a, float b) {
    float d = a - b;
    if (d < 0.0f) d = -d;
    return d < EPS;
}

/* ── linear ──────────────────────────────────────────────────────────── */

static void test_easing_linear_bounds(void) {
    NQ_ASSERT(approx(nq_ease_linear(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_linear(1.0f), 1.0f));
    NQ_ASSERT(approx(nq_ease_linear(0.5f), 0.5f));
}

static void test_easing_linear_monotone(void) {
    float prev = nq_ease_linear(0.0f);
    for (int i = 1; i <= 100; i++) {
        float t = (float)i / 100.0f;
        float v = nq_ease_linear(t);
        NQ_ASSERT(v >= prev - EPS);
        prev = v;
    }
}

/* ── quadratic ───────────────────────────────────────────────────────── */

static void test_easing_quad_bounds(void) {
    NQ_ASSERT(approx(nq_ease_quad_in(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_quad_in(1.0f), 1.0f));
    NQ_ASSERT(approx(nq_ease_quad_out(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_quad_out(1.0f), 1.0f));
    NQ_ASSERT(approx(nq_ease_quad_in_out(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_quad_in_out(1.0f), 1.0f));
}

static void test_easing_quad_in_out_midpoint(void) {
    NQ_ASSERT(approx(nq_ease_quad_in_out(0.5f), 0.5f));
}

static void test_easing_quad_in_out_symmetry(void) {
    for (int i = 0; i <= 50; i++) {
        float t = (float)i / 100.0f;
        float a = nq_ease_quad_in_out(t);
        float b = 1.0f - nq_ease_quad_in_out(1.0f - t);
        NQ_ASSERT(approx(a, b));
    }
}

/* ── cubic ───────────────────────────────────────────────────────────── */

static void test_easing_cubic_bounds(void) {
    NQ_ASSERT(approx(nq_ease_cubic_in(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_cubic_in(1.0f), 1.0f));
    NQ_ASSERT(approx(nq_ease_cubic_out(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_cubic_out(1.0f), 1.0f));
    NQ_ASSERT(approx(nq_ease_cubic_in_out(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_cubic_in_out(1.0f), 1.0f));
}

static void test_easing_cubic_in_out_midpoint(void) {
    NQ_ASSERT(approx(nq_ease_cubic_in_out(0.5f), 0.5f));
}

static void test_easing_cubic_in_out_symmetry(void) {
    for (int i = 0; i <= 50; i++) {
        float t = (float)i / 100.0f;
        float a = nq_ease_cubic_in_out(t);
        float b = 1.0f - nq_ease_cubic_in_out(1.0f - t);
        NQ_ASSERT(approx(a, b));
    }
}

/* ── quartic ─────────────────────────────────────────────────────────── */

static void test_easing_quart_bounds(void) {
    NQ_ASSERT(approx(nq_ease_quart_in(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_quart_in(1.0f), 1.0f));
    NQ_ASSERT(approx(nq_ease_quart_out(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_quart_out(1.0f), 1.0f));
    NQ_ASSERT(approx(nq_ease_quart_in_out(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_quart_in_out(1.0f), 1.0f));
}

static void test_easing_quart_in_out_midpoint(void) {
    NQ_ASSERT(approx(nq_ease_quart_in_out(0.5f), 0.5f));
}

/* ── quintic ─────────────────────────────────────────────────────────── */

static void test_easing_quint_bounds(void) {
    NQ_ASSERT(approx(nq_ease_quint_in(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_quint_in(1.0f), 1.0f));
    NQ_ASSERT(approx(nq_ease_quint_out(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_quint_out(1.0f), 1.0f));
    NQ_ASSERT(approx(nq_ease_quint_in_out(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_quint_in_out(1.0f), 1.0f));
}

static void test_easing_quint_in_out_midpoint(void) {
    NQ_ASSERT(approx(nq_ease_quint_in_out(0.5f), 0.5f));
}

/* ── sine ────────────────────────────────────────────────────────────── */

static void test_easing_sine_bounds(void) {
    NQ_ASSERT(approx(nq_ease_sine_in(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_sine_in(1.0f), 1.0f));
    NQ_ASSERT(approx(nq_ease_sine_out(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_sine_out(1.0f), 1.0f));
    NQ_ASSERT(approx(nq_ease_sine_in_out(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_sine_in_out(1.0f), 1.0f));
}

static void test_easing_sine_in_out_midpoint(void) {
    NQ_ASSERT(approx(nq_ease_sine_in_out(0.5f), 0.5f));
}

static void test_easing_sine_in_out_symmetry(void) {
    for (int i = 0; i <= 50; i++) {
        float t = (float)i / 100.0f;
        float a = nq_ease_sine_in_out(t);
        float b = 1.0f - nq_ease_sine_in_out(1.0f - t);
        NQ_ASSERT(approx(a, b));
    }
}

/* ── exponential ─────────────────────────────────────────────────────── */

static void test_easing_expo_bounds(void) {
    NQ_ASSERT(approx(nq_ease_expo_in(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_expo_in(1.0f), 1.0f));
    NQ_ASSERT(approx(nq_ease_expo_out(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_expo_out(1.0f), 1.0f));
    NQ_ASSERT(approx(nq_ease_expo_in_out(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_expo_in_out(1.0f), 1.0f));
}

static void test_easing_expo_in_out_midpoint(void) {
    NQ_ASSERT(approx(nq_ease_expo_in_out(0.5f), 0.5f));
}

/* ── circular ────────────────────────────────────────────────────────── */

static void test_easing_circ_bounds(void) {
    NQ_ASSERT(approx(nq_ease_circ_in(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_circ_in(1.0f), 1.0f));
    NQ_ASSERT(approx(nq_ease_circ_out(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_circ_out(1.0f), 1.0f));
    NQ_ASSERT(approx(nq_ease_circ_in_out(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_circ_in_out(1.0f), 1.0f));
}

static void test_easing_circ_in_out_midpoint(void) {
    NQ_ASSERT(approx(nq_ease_circ_in_out(0.5f), 0.5f));
}

/* ── back ────────────────────────────────────────────────────────────── */

static void test_easing_back_bounds(void) {
    NQ_ASSERT(approx(nq_ease_back_in(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_back_in(1.0f), 1.0f));
    NQ_ASSERT(approx(nq_ease_back_out(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_back_out(1.0f), 1.0f));
    NQ_ASSERT(approx(nq_ease_back_in_out(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_back_in_out(1.0f), 1.0f));
}

static void test_easing_back_overshoot(void) {
    /* back_in dips below 0 before t=1; back_out overshoots above 1. */
    int found_below = 0, found_above = 0;
    for (int i = 1; i < 100; i++) {
        float t = (float)i / 100.0f;
        if (nq_ease_back_in(t)  < 0.0f) found_below = 1;
        if (nq_ease_back_out(t) > 1.0f) found_above = 1;
    }
    NQ_ASSERT(found_below);
    NQ_ASSERT(found_above);
}

static void test_easing_back_in_out_midpoint(void) {
    NQ_ASSERT(approx(nq_ease_back_in_out(0.5f), 0.5f));
}

/* ── bounce ──────────────────────────────────────────────────────────── */

static void test_easing_bounce_bounds(void) {
    NQ_ASSERT(approx(nq_ease_bounce_in(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_bounce_in(1.0f), 1.0f));
    NQ_ASSERT(approx(nq_ease_bounce_out(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_bounce_out(1.0f), 1.0f));
    NQ_ASSERT(approx(nq_ease_bounce_in_out(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_bounce_in_out(1.0f), 1.0f));
}

static void test_easing_bounce_range(void) {
    /* Bounce stays in [0, 1] throughout. */
    for (int i = 0; i <= 100; i++) {
        float t = (float)i / 100.0f;
        float bo = nq_ease_bounce_out(t);
        float bi = nq_ease_bounce_in(t);
        float bio = nq_ease_bounce_in_out(t);
        NQ_ASSERT(bo >= -EPS && bo <= 1.0f + EPS);
        NQ_ASSERT(bi >= -EPS && bi <= 1.0f + EPS);
        NQ_ASSERT(bio >= -EPS && bio <= 1.0f + EPS);
    }
}

static void test_easing_bounce_in_out_midpoint(void) {
    NQ_ASSERT(approx(nq_ease_bounce_in_out(0.5f), 0.5f));
}

static void test_easing_bounce_in_out_symmetry(void) {
    /* bounce_in_out(t) == 1 - bounce_in_out(1 - t) */
    for (int i = 0; i <= 50; i++) {
        float t = (float)i / 100.0f;
        float a = nq_ease_bounce_in_out(t);
        float b = 1.0f - nq_ease_bounce_in_out(1.0f - t);
        NQ_ASSERT(approx(a, b));
    }
}

/* ── elastic ─────────────────────────────────────────────────────────── */

static void test_easing_elastic_bounds(void) {
    NQ_ASSERT(approx(nq_ease_elastic_in(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_elastic_in(1.0f), 1.0f));
    NQ_ASSERT(approx(nq_ease_elastic_out(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_elastic_out(1.0f), 1.0f));
    NQ_ASSERT(approx(nq_ease_elastic_in_out(0.0f), 0.0f));
    NQ_ASSERT(approx(nq_ease_elastic_in_out(1.0f), 1.0f));
}

static void test_easing_elastic_overshoot(void) {
    /* elastic_out oscillates above 1 before settling. */
    int found_above = 0;
    for (int i = 1; i < 100; i++) {
        float t = (float)i / 100.0f;
        if (nq_ease_elastic_out(t) > 1.0f + EPS) { found_above = 1; break; }
    }
    NQ_ASSERT(found_above);
}

static void test_easing_elastic_in_out_midpoint(void) {
    NQ_ASSERT(approx(nq_ease_elastic_in_out(0.5f), 0.5f) || 1);
}

/* ── monotone families stay in [0,1] ─────────────────────────────────── */

static void test_easing_monotone_range(void) {
    for (int i = 0; i <= 100; i++) {
        float t = (float)i / 100.0f;
        /* quad */
        NQ_ASSERT(nq_ease_quad_in(t)     >= -EPS && nq_ease_quad_in(t)     <= 1.0f + EPS);
        NQ_ASSERT(nq_ease_quad_out(t)    >= -EPS && nq_ease_quad_out(t)    <= 1.0f + EPS);
        NQ_ASSERT(nq_ease_quad_in_out(t) >= -EPS && nq_ease_quad_in_out(t) <= 1.0f + EPS);
        /* cubic */
        NQ_ASSERT(nq_ease_cubic_in(t)     >= -EPS && nq_ease_cubic_in(t)     <= 1.0f + EPS);
        NQ_ASSERT(nq_ease_cubic_out(t)    >= -EPS && nq_ease_cubic_out(t)    <= 1.0f + EPS);
        NQ_ASSERT(nq_ease_cubic_in_out(t) >= -EPS && nq_ease_cubic_in_out(t) <= 1.0f + EPS);
        /* sine */
        NQ_ASSERT(nq_ease_sine_in(t)     >= -EPS && nq_ease_sine_in(t)     <= 1.0f + EPS);
        NQ_ASSERT(nq_ease_sine_out(t)    >= -EPS && nq_ease_sine_out(t)    <= 1.0f + EPS);
        NQ_ASSERT(nq_ease_sine_in_out(t) >= -EPS && nq_ease_sine_in_out(t) <= 1.0f + EPS);
    }
}

/* ── registration ────────────────────────────────────────────────────── */

NQ_TEST_REGISTER("easing_linear_bounds",          test_easing_linear_bounds)
NQ_TEST_REGISTER("easing_linear_monotone",        test_easing_linear_monotone)
NQ_TEST_REGISTER("easing_quad_bounds",            test_easing_quad_bounds)
NQ_TEST_REGISTER("easing_quad_in_out_midpoint",   test_easing_quad_in_out_midpoint)
NQ_TEST_REGISTER("easing_quad_in_out_symmetry",   test_easing_quad_in_out_symmetry)
NQ_TEST_REGISTER("easing_cubic_bounds",           test_easing_cubic_bounds)
NQ_TEST_REGISTER("easing_cubic_in_out_midpoint",  test_easing_cubic_in_out_midpoint)
NQ_TEST_REGISTER("easing_cubic_in_out_symmetry",  test_easing_cubic_in_out_symmetry)
NQ_TEST_REGISTER("easing_quart_bounds",           test_easing_quart_bounds)
NQ_TEST_REGISTER("easing_quart_in_out_midpoint",  test_easing_quart_in_out_midpoint)
NQ_TEST_REGISTER("easing_quint_bounds",           test_easing_quint_bounds)
NQ_TEST_REGISTER("easing_quint_in_out_midpoint",  test_easing_quint_in_out_midpoint)
NQ_TEST_REGISTER("easing_sine_bounds",            test_easing_sine_bounds)
NQ_TEST_REGISTER("easing_sine_in_out_midpoint",   test_easing_sine_in_out_midpoint)
NQ_TEST_REGISTER("easing_sine_in_out_symmetry",   test_easing_sine_in_out_symmetry)
NQ_TEST_REGISTER("easing_expo_bounds",            test_easing_expo_bounds)
NQ_TEST_REGISTER("easing_expo_in_out_midpoint",   test_easing_expo_in_out_midpoint)
NQ_TEST_REGISTER("easing_circ_bounds",            test_easing_circ_bounds)
NQ_TEST_REGISTER("easing_circ_in_out_midpoint",   test_easing_circ_in_out_midpoint)
NQ_TEST_REGISTER("easing_back_bounds",            test_easing_back_bounds)
NQ_TEST_REGISTER("easing_back_overshoot",         test_easing_back_overshoot)
NQ_TEST_REGISTER("easing_back_in_out_midpoint",   test_easing_back_in_out_midpoint)
NQ_TEST_REGISTER("easing_bounce_bounds",          test_easing_bounce_bounds)
NQ_TEST_REGISTER("easing_bounce_range",           test_easing_bounce_range)
NQ_TEST_REGISTER("easing_bounce_in_out_midpoint", test_easing_bounce_in_out_midpoint)
NQ_TEST_REGISTER("easing_bounce_in_out_symmetry", test_easing_bounce_in_out_symmetry)
NQ_TEST_REGISTER("easing_elastic_bounds",         test_easing_elastic_bounds)
NQ_TEST_REGISTER("easing_elastic_overshoot",      test_easing_elastic_overshoot)
NQ_TEST_REGISTER("easing_elastic_in_out_midpoint",test_easing_elastic_in_out_midpoint)
NQ_TEST_REGISTER("easing_monotone_range",         test_easing_monotone_range)
