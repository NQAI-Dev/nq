#include <nq/clock.h>
#include "test_main.c"

/* Mock time source. Tests advance this directly to avoid sleep(). */
static uint64_t mock_time_ns = 1000000000ull;  /* start at 1s */
static uint64_t mock_now(void) { return mock_time_ns; }
static void advance_ms(uint64_t ms) { mock_time_ns += ms * 1000000ull; }

static void test_clock_create_destroy(void) {
    NqClock *c = nq_clock_create();
    NQ_ASSERT(c != NULL);
    nq_clock_destroy(c);
    nq_clock_destroy(NULL);  /* safe no-op */
}

static void test_clock_first_tick_delta_zero(void) {
    nq_clock_set_now_fn(mock_now);
    mock_time_ns = 1000000000ull;
    NqClock *c = nq_clock_create();
    NqFrameTime ft;
    /* On the very first tick no time has elapsed since start_ns — dt == 0. */
    NQ_ASSERT_EQ(nq_clock_tick(c, &ft), 0);
    NQ_ASSERT_EQ(ft.frame_index, 1);
    NQ_ASSERT(ft.delta_seconds == 0.0);
    nq_clock_destroy(c);
}

static void test_clock_tick_delta_16ms(void) {
    /* Standard 60Hz frame budget: 16.6ms. */
    nq_clock_set_now_fn(mock_now);
    mock_time_ns = 1000000000ull;
    NqClock *c = nq_clock_create();
    NqFrameTime ft;
    (void)nq_clock_tick(c, &ft);   /* discard first tick */
    advance_ms(16);               /* ~16ms between ticks */
    NQ_ASSERT_EQ(nq_clock_tick(c, &ft), 0);
    NQ_ASSERT_EQ(ft.frame_index, 2);
    /* tolerance for clock_gettime rounding: 15-17ms */
    NQ_ASSERT(ft.delta_seconds > 0.015 && ft.delta_seconds < 0.0175);
    NQ_ASSERT(ft.elapsed_seconds > 0.015);
    nq_clock_destroy(c);
}

static void test_clock_elapsed_accumulates(void) {
    nq_clock_set_now_fn(mock_now);
    mock_time_ns = 0;
    NqClock *c = nq_clock_create();
    NqFrameTime ft;
    for (int i = 0; i < 10; i++) {
        advance_ms(16);
        (void)nq_clock_tick(c, &ft);
    }
    /* 10 ticks at 16ms each ~= 160ms total elapsed */
    NQ_ASSERT_EQ(ft.frame_index, 10);
    NQ_ASSERT(ft.elapsed_seconds > 0.155 && ft.elapsed_seconds < 0.165);
    nq_clock_destroy(c);
}

static void test_clock_set_now_fn_default(void) {
    /* Passing NULL resets to default (real clock_gettime). Just verify
     * no crash and that create+destroy cycle works under the real source. */
    nq_clock_set_now_fn(NULL);
    NqClock *c = nq_clock_create();
    NQ_ASSERT(c != NULL);
    NqFrameTime ft;
    (void)nq_clock_tick(c, &ft);
    NQ_ASSERT(ft.delta_seconds >= 0.0);
    nq_clock_destroy(c);
}

static void test_clock_null_safe(void) {
    NqClock *c = nq_clock_create();
    /* tick with NULL out is fine */
    NQ_ASSERT_EQ(nq_clock_tick(c, NULL), 0);
    /* tick with NULL clock fails */
    NQ_ASSERT_EQ(nq_clock_tick(NULL, NULL), -1);
    nq_clock_destroy(c);
}

NQ_TEST_REGISTER("clock_create_destroy",          test_clock_create_destroy)
NQ_TEST_REGISTER("clock_first_tick_delta_zero",   test_clock_first_tick_delta_zero)
NQ_TEST_REGISTER("clock_tick_delta_16ms",          test_clock_tick_delta_16ms)
NQ_TEST_REGISTER("clock_elapsed_accumulates",      test_clock_elapsed_accumulates)
NQ_TEST_REGISTER("clock_set_now_fn_default",      test_clock_set_now_fn_default)
NQ_TEST_REGISTER("clock_null_safe",               test_clock_null_safe)
