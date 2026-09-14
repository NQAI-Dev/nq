#include "nq/clock.h"

#include <stdlib.h>
#include <time.h>

static uint64_t nq_clock_default_now_ns(void) {
    struct timespec ts;
    /* CLOCK_MONOTONIC is unaffected by wall-clock changes (NTP / DST) and
     * is the right source for frame timing on POSIX. */
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
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
