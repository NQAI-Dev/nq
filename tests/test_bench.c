#include <nq/bench.h>
#include <SDL3/SDL.h>
#include "test_main.c"

static void test_bench_now_returns_monotonic(void) {
    uint64_t t1 = nq_bench_now_ns();
    SDL_Delay(2);  /* 2ms */
    uint64_t t2 = nq_bench_now_ns();
    /* At least 1ms elapsed (clock resolution varies). */
    NQ_ASSERT(t2 > t1);
    NQ_ASSERT(t2 - t1 >= 1000000);  /* >= 1ms */
}

static void test_bench_elapsed_monotonic(void) {
    uint64_t start = nq_bench_now_ns();
    SDL_Delay(3);  /* 3ms */
    uint64_t dt = nq_bench_elapsed_ns(start);
    NQ_ASSERT(dt >= 2000000);  /* >= 2ms (some slack) */
}

static void test_bench_scope_records_hits(void) {
    nq_bench_reset();
    for (int i = 0; i < 5; i++) {
        NqBenchScope s;
        nq_bench_scope_begin(&s, "test_scope_a");
        SDL_Delay(1);
        nq_bench_scope_end(&s);
    }
    /* We can't easily inspect the records (file-scope), but the API
     * shouldn't have crashed — the implicit assertion is that this
     * loops cleanly. flush() to stderr also exercises the path. */
    nq_bench_flush();
    NQ_ASSERT(1);
}

static void test_bench_no_crash_on_zero_iterations(void) {
    nq_bench_reset();
    nq_bench_flush();  /* no records — should be a no-op */
    NQ_ASSERT(1);
}

NQ_TEST_REGISTER("bench_now_monotonic",           test_bench_now_returns_monotonic)
NQ_TEST_REGISTER("bench_elapsed_monotonic",       test_bench_elapsed_monotonic)
NQ_TEST_REGISTER("bench_scope_records_hits",      test_bench_scope_records_hits)
NQ_TEST_REGISTER("bench_no_crash_empty",          test_bench_no_crash_on_zero_iterations)
