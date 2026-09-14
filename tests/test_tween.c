#include <nq/tween.h>
#include "test_main.c"

/* Small epsilon for float comparison. */
#define NQ_EPS 1e-5f
#define NQ_FE(a, b) ((a) - (b) < NQ_EPS && (b) - (a) < NQ_EPS)

static void test_linear_endpoints(void) {
    NQ_FE(nq_ease_linear(0.0f), 0.0f);
    NQ_FE(nq_ease_linear(1.0f), 1.0f);
    NQ_FE(nq_ease_linear(0.5f), 0.5f);
}

static void test_quad_in_endpoints(void) {
    /* All easings must satisfy f(0)==0, f(1)==1 (canonical convention). */
    NQ_FE(nq_ease_quad_in(0.0f), 0.0f);
    NQ_FE(nq_ease_quad_in(1.0f), 1.0f);
    NQ_FE(nq_ease_quad_in(0.5f), 0.25f);   /* 0.5^2 */
}

static void test_quad_in_monotonic(void) {
    /* quad_in is strictly increasing on [0,1] */
    float prev = 0.0f;
    for (int i = 1; i <= 100; i++) {
        float t = (float)i / 100.0f;
        float v = nq_ease_quad_in(t);
        NQ_ASSERT(v > prev - NQ_EPS);  /* increasing */
        prev = v;
    }
}

static void test_quad_out_endpoints(void) {
    NQ_FE(nq_ease_quad_out(0.0f), 0.0f);
    NQ_FE(nq_ease_quad_out(1.0f), 1.0f);
    /* quad_out is steeper near 1: f(0.5) should be ~0.75 (not 0.5 as in linear) */
    NQ_FE(nq_ease_quad_out(0.5f), 0.75f);  /* 0.5 * (2 - 0.5) */
}

static void test_quad_in_out_endpoints(void) {
    NQ_FE(nq_ease_quad_in_out(0.0f), 0.0f);
    NQ_FE(nq_ease_quad_in_out(1.0f), 1.0f);
    NQ_FE(nq_ease_quad_in_out(0.5f), 0.5f);   /* midpoint */
}

static void test_cubic_endpoints(void) {
    NQ_FE(nq_ease_cubic_in(0.0f), 0.0f);
    NQ_FE(nq_ease_cubic_in(1.0f), 1.0f);
    NQ_FE(nq_ease_cubic_in(0.5f), 0.125f);   /* 0.5^3 */
    NQ_FE(nq_ease_cubic_out(0.0f), 0.0f);
    NQ_FE(nq_ease_cubic_out(1.0f), 1.0f);
}

static void test_sine_endpoints(void) {
    NQ_FE(nq_ease_sine_in(0.0f),  0.0f);
    NQ_FE(nq_ease_sine_in(1.0f),  1.0f);
    NQ_FE(nq_ease_sine_out(0.0f), 0.0f);
    NQ_FE(nq_ease_sine_out(1.0f), 1.0f);
    NQ_FE(nq_ease_sine_in_out(0.0f),  0.0f);
    NQ_FE(nq_ease_sine_in_out(1.0f),  1.0f);
    NQ_FE(nq_ease_sine_in_out(0.5f),  0.5f);   /* sin/cos symmetric */
}

static void test_expo_endpoints(void) {
    NQ_FE(nq_ease_expo_in(0.0f), 0.0f);
    NQ_FE(nq_ease_expo_in(1.0f), 1.0f);
    NQ_FE(nq_ease_expo_out(0.0f), 0.0f);
    NQ_FE(nq_ease_expo_out(1.0f), 1.0f);
}

static void test_ease_dispatch_endpoints(void) {
    /* The dispatch nq_ease() should reach the same answer as named accessors
     * for both endpoints, for every kind. */
    NqEaseKind kinds[] = {
        NQ_EASE_LINEAR, NQ_EASE_QUAD_IN, NQ_EASE_QUAD_OUT, NQ_EASE_QUAD_IN_OUT,
        NQ_EASE_CUBIC_IN, NQ_EASE_CUBIC_OUT, NQ_EASE_CUBIC_IN_OUT,
        NQ_EASE_SINE_IN, NQ_EASE_SINE_OUT, NQ_EASE_SINE_IN_OUT,
        NQ_EASE_EXPO_IN, NQ_EASE_EXPO_OUT,
    };
    for (size_t i = 0; i < sizeof(kinds) / sizeof(kinds[0]); i++) {
        NqEaseKind k = kinds[i];
        /* At t=0, all easings must return 0 (or near-0 for expo). */
        float v0 = nq_ease(k, 0.0f);
        NQ_ASSERT(v0 >= -NQ_EPS && v0 <= NQ_EPS);
        /* At t=1, all easings must return 1. */
        float v1 = nq_ease(k, 1.0f);
        NQ_ASSERT(v1 >= 1.0f - NQ_EPS && v1 <= 1.0f + NQ_EPS);
    }
}

static void test_ease_clamps_out_of_range(void) {
    /* nq_ease() clamps t to [0,1]; out-of-range t must still give f(0)/f(1). */
    NQ_FE(nq_ease(NQ_EASE_QUAD_IN, -0.5f), 0.0f);
    NQ_FE(nq_ease(NQ_EASE_QUAD_IN,  1.7f), 1.0f);
    NQ_FE(nq_ease(NQ_EASE_LINEAR, -1.0f), 0.0f);
    NQ_FE(nq_ease(NQ_EASE_LINEAR,  2.0f), 1.0f);
}

static void test_lerp_endpoints(void) {
    NQ_FE(nq_lerp(10.0f, 20.0f, 0.0f), 10.0f);
    NQ_FE(nq_lerp(10.0f, 20.0f, 1.0f), 20.0f);
    NQ_FE(nq_lerp(0.0f, 100.0f, 0.5f), 50.0f);
    /* negative t clamps to 0 */
    NQ_FE(nq_lerp(10.0f, 20.0f, -0.5f), 10.0f);
    /* >1 t clamps to 1 */
    NQ_FE(nq_lerp(10.0f, 20.0f, 1.5f), 20.0f);
}

static void test_ease_lerp_matches_named(void) {
    /* nq_ease_lerp(kind, a, b, t) must equal nq_lerp(a, b, nq_ease(kind, t)). */
    float a = 0.0f, b = 100.0f;
    for (int i = 0; i <= 10; i++) {
        float t = (float)i / 10.0f;
        float expected = nq_lerp(a, b, nq_ease_quad_in(t));
        float actual   = nq_ease_lerp(NQ_EASE_QUAD_IN, a, b, t);
        NQ_FE(actual, expected);
    }
    /* Try a different kind — sine out */
    float v = nq_ease_lerp(NQ_EASE_SINE_OUT, -5.0f, 5.0f, 0.25f);
    NQ_FE(v, nq_lerp(-5.0f, 5.0f, nq_ease_sine_out(0.25f)));
}

NQ_TEST_REGISTER("linear_endpoints",            test_linear_endpoints);
NQ_TEST_REGISTER("quad_in_endpoints",           test_quad_in_endpoints);
NQ_TEST_REGISTER("quad_in_monotonic",          test_quad_in_monotonic);
NQ_TEST_REGISTER("quad_out_endpoints",          test_quad_out_endpoints);
NQ_TEST_REGISTER("quad_in_out_endpoints",       test_quad_in_out_endpoints);
NQ_TEST_REGISTER("cubic_endpoints",             test_cubic_endpoints);
NQ_TEST_REGISTER("sine_endpoints",              test_sine_endpoints);
NQ_TEST_REGISTER("expo_endpoints",              test_expo_endpoints);
NQ_TEST_REGISTER("ease_dispatch_endpoints",     test_ease_dispatch_endpoints);
NQ_TEST_REGISTER("ease_clamps_out_of_range",    test_ease_clamps_out_of_range);
NQ_TEST_REGISTER("lerp_endpoints",              test_lerp_endpoints);
NQ_TEST_REGISTER("ease_lerp_matches_named",     test_ease_lerp_matches_named);
