/*
 * nq — frame clock.
 *
 * Monotonic high-resolution time source + per-frame snapshot. Single
 * instance per engine context is enough; no need for thread-safe caching
 * because games are single-threaded at the engine level (workers have
 * their own clock).
 *
 * Time source is hookable for testing: nq_clock_set_now_fn() lets unit
 * tests advance time deterministically without sleep(). Pass NULL to
 * restore the default clock_gettime(CLOCK_MONOTONIC).
 */
#ifndef NQ_CLOCK_H
#define NQ_CLOCK_H

#include <stdint.h>

typedef struct NqClock NqClock;

typedef struct {
    double   delta_seconds;   /* time since previous tick */
    double   elapsed_seconds; /* total since nq_clock_create */
    uint64_t frame_index;     /* monotonic, 1-based at first tick */
} NqFrameTime;

NqClock *nq_clock_create(void);
void     nq_clock_destroy(NqClock *c);
int      nq_clock_tick(NqClock *c, NqFrameTime *out);

/* Override the now() source. fn() must return monotonic non-decreasing
 * nanoseconds. Passing NULL restores the default clock_gettime source. */
typedef uint64_t (*nq_clock_now_fn_t)(void);
void nq_clock_set_now_fn(nq_clock_now_fn_t fn);

#endif /* NQ_CLOCK_H */
