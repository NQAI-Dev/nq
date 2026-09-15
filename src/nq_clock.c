#include "nq/clock.h"

#include <SDL3/SDL.h>

#include <stdlib.h>

/* Cross-platform monotonic time source via SDL3. SDL_GetPerformanceCounter
 * returns SDL3 ticks (CPU-specific high-res), SDL_GetPerformanceFrequency
 * returns ticks-per-second. Conversion to nanoseconds is exact up to
 * 2^32 / freq before precision loss; on a 3 GHz CPU with SDL3 freq ≈
 * 10^9 Hz, that's hours of continuous runtime before precision drift. */
static uint64_t nq_clock_default_now_ns(void) {
    static Uint64 freq = 0;
    if (freq == 0) {
        freq = SDL_GetPerformanceFrequency();
    }
    Uint64 ticks = SDL_GetPerformanceCounter();
    /* Round to nearest ns to keep frame deltas stable on platforms where
     * perf frequency is a round multiple of 10^9. */
    return (uint64_t)((double)ticks * 1e9 / (double)freq + 0.5);
}

static nq_clock_now_fn_t s_now_fn = nq_clock_default_now_ns;

void nq_clock_set_now_fn(nq_clock_now_fn_t fn) {
    s_now_fn = fn ? fn : nq_clock_default_now_ns;
}

struct NqClock {
    uint64_t start_ns;
    uint64_t last_ns;
    uint64_t frame_index;
};

NqClock *nq_clock_create(void) {
    NqClock *c = calloc(1, sizeof(NqClock));
    if (!c) {
        return NULL;
    }
    uint64_t now = s_now_fn();
    c->start_ns = now;
    c->last_ns = now;
    return c;
}

void nq_clock_destroy(NqClock *c) {
    free(c);
}

int nq_clock_tick(NqClock *c, NqFrameTime *out) {
    if (!c) {
        return -1;
    }
    uint64_t now = s_now_fn();
    uint64_t dt_ns = now >= c->last_ns ? (now - c->last_ns) : 0;
    c->last_ns = now;
    c->frame_index++;
    if (out) {
        out->delta_seconds   = (double)dt_ns / 1e9;
        out->elapsed_seconds = (double)(now - c->start_ns) / 1e9;
        out->frame_index     = c->frame_index;
    }
    return 0;
}
