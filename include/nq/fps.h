/*
 * nq — FPS counter (frames per second sampler).
 *
 * Computes average FPS over a rolling 1-second window. Useful for
 * in-game debug overlays and for runtime regression detection in
 * CI builds (drift > 10% between releases triggers a warning).
 *
 * Pattern: call nq_fps_counter_tick() once per render frame with
 * the elapsed_seconds from the NqFrameTime snapshot, then call
 * nq_fps_counter_get() whenever you need to render the number.
 * The counter only recomputes FPS when a full 1.0-second window
 * has elapsed; between recomputations get() returns the most
 * recent value (zero before the first full window completes).
 *
 * Engine-internal — no SDL3 / GL / platform headers. Same
 * deterministic-clock pattern as NqClock for unit testing.
 */
#ifndef NQ_FPS_H
#define NQ_FPS_H

typedef struct {
    int    frame_count;    /* frames in the current 1-second window */
    double window_start;   /* elapsed_seconds at the start of the window */
    int    last_fps;       /* most recent computed FPS (0 until first window completes) */
    double last_window;    /* most recent window duration in seconds (≤ 1.0 when short frames) */
} NqFpsCounter;

/* Zero-initialise. Calling init() on an already-used counter
 * resets the rolling window — useful when transitioning between
 * scenes that have wildly different render costs. */
void nq_fps_counter_init(NqFpsCounter *c);

/* Advance the counter by one frame at elapsed_seconds (from
 * NqFrameTime.elapsed_seconds). When the current window reaches
 * ≥ 1.0 second, recompute last_fps and reset the window.
 *
 * No-op when c is NULL. elapsed_seconds may be 0 (first frame)
 * or any positive value (no upper bound — large gaps just mean
 * a longer initial window). */
void nq_fps_counter_tick(NqFpsCounter *c, double elapsed_seconds);

/* Returns the most recent computed FPS (rounded to nearest int),
 * or 0 until the first 1-second window completes. */
int nq_fps_counter_get(const NqFpsCounter *c);

#endif /* NQ_FPS_H */
