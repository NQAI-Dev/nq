/*
 * nq — bench.
 *
 * Header-only high-resolution timer for ad-hoc profiling. Wraps
 * SDL_GetPerformanceCounter / SDL_GetPerformanceFrequency so the same
 * time source is used everywhere (nq_clock uses it too).
 *
 * Usage — manual:
 *   uint64_t t = nq_bench_now_ns();
 *   ...work...
 *   uint64_t elapsed_ns = nq_bench_elapsed_ns(t);
 *
 * Usage — RAII-style (gcc/clang):
 *   NQ_BENCH_SCOPE("expensive fn") {
 *       ...work...
 *   }
 *   nq_bench_flush();  // prints accumulated scopes
 *
 * Output goes to stderr so it can be redirected separately from the
 * engine's normal log output. When NQ_BENCH=0 is set in the env, the
 * macros compile to nothing — production builds pay zero cost.
 */
#ifndef NQ_BENCH_H
#define NQ_BENCH_H

#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdio.h>

#ifndef NQ_BENCH
#  ifdef NDEBUG
#    define NQ_BENCH 0
#  else
#    define NQ_BENCH 1
#  endif
#endif

#if NQ_BENCH

static inline uint64_t nq_bench_now_ns(void) {
    static Uint64 freq = 0;
    if (freq == 0) {
        freq = SDL_GetPerformanceFrequency();
    }
    Uint64 ticks = SDL_GetPerformanceCounter();
    return (uint64_t)((double)ticks * 1e9 / (double)freq + 0.5);
}

static inline uint64_t nq_bench_elapsed_ns(uint64_t start_ns) {
    uint64_t now = nq_bench_now_ns();
    return now >= start_ns ? (now - start_ns) : 0;
}

#define NQ_BENCH_MAX_SCOPES 64

typedef struct {
    const char *name;
    uint64_t start_ns;
    uint64_t total_ns;
    uint64_t hits;
} NqBenchRecord;

static NqBenchRecord nq_bench_records[NQ_BENCH_MAX_SCOPES];
static int nq_bench_record_count = 0;

static inline int nq_bench_find_or_add_record(const char *name) {
    if (!name) return -1;
    /* Look up first — a hot scope reuses the same record slot. */
    for (int i = 0; i < nq_bench_record_count; i++) {
        if (nq_bench_records[i].name == name) return i;
    }
    if (nq_bench_record_count >= NQ_BENCH_MAX_SCOPES) return -1;
    int idx = nq_bench_record_count++;
    nq_bench_records[idx].name = name;
    nq_bench_records[idx].total_ns = 0;
    nq_bench_records[idx].hits = 0;
    return idx;
}

typedef struct {
    const char *name;
    int   record_idx;
    uint64_t start_ns;
} NqBenchScope;

static inline void nq_bench_scope_begin(NqBenchScope *s, const char *name) {
    int idx = nq_bench_find_or_add_record(name);
    s->name = name;
    s->record_idx = idx;
    s->start_ns = (idx >= 0) ? nq_bench_now_ns() : 0;
}

static inline void nq_bench_scope_end(NqBenchScope *s) {
    if (s->record_idx < 0) return;
    uint64_t dt = nq_bench_elapsed_ns(s->start_ns);
    nq_bench_records[s->record_idx].total_ns += dt;
    nq_bench_records[s->record_idx].hits++;
}

static inline void nq_bench_flush(void) {
    if (nq_bench_record_count == 0) return;
    fprintf(stderr, "[nq-bench] accumulated scope timings:\n");
    for (int i = 0; i < nq_bench_record_count; i++) {
        const NqBenchRecord *r = &nq_bench_records[i];
        if (r->hits == 0) continue;
        double avg = (double)r->total_ns / (double)r->hits;
        fprintf(stderr, "  %-32s %llu hits, %llu ns total, %.1f ns avg\n",
                r->name,
                (unsigned long long)r->hits,
                (unsigned long long)r->total_ns,
                avg);
    }
}

static inline void nq_bench_reset(void) {
    for (int i = 0; i < nq_bench_record_count; i++) {
        nq_bench_records[i].total_ns = 0;
        nq_bench_records[i].hits = 0;
    }
}

/* RAII-style scope on gcc/clang: the cleanup attribute runs the end fn
 * when the scope variable goes out of scope, even on early return. */
#if defined(__GNUC__) || defined(__clang__)
#  define NQ_BENCH_SCOPE(name)                                              \
       NqBenchScope nq_bench__scope_##__LINE__ __attribute__((cleanup(nq_bench_scope_end))); \
       nq_bench_scope_begin(&nq_bench__scope_##__LINE__, name)
#else
/* MSVC: explicit scope + manual end. Caller writes
 *   NQ_BENCH_SCOPE_BEGIN("name")  / ...work...  / NQ_BENCH_SCOPE_END()
 */
#  define NQ_BENCH_SCOPE(name) /* no-op on MSVC; use explicit pair below */
static inline void nq_bench_scope_pair(const char *name,
                                       NqBenchScope *scope,
                                       int *active) {
    (void)active;
    nq_bench_scope_begin(scope, name);
}
#  define NQ_BENCH_SCOPE_BEGIN(name) NqBenchScope nq_bench__scope; nq_bench_scope_begin(&nq_bench__scope, name)
#  define NQ_BENCH_SCOPE_END()       nq_bench_scope_end(&nq_bench__scope)
#endif

#else  /* !NQ_BENCH */

/* Compiled-out no-op versions for production builds. */
static inline uint64_t nq_bench_now_ns(void) { return 0; }
static inline uint64_t nq_bench_elapsed_ns(uint64_t start_ns) {
    (void)start_ns; return 0;
}
static inline void nq_bench_flush(void) {}
static inline void nq_bench_reset(void) {}
#  define NQ_BENCH_SCOPE(name) ((void)0)

#endif /* NQ_BENCH */

#endif /* NQ_BENCH_H */
