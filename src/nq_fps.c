#include "nq/fps.h"

void nq_fps_counter_init(NqFpsCounter *c) {
    if (!c) return;
    c->frame_count = 0;
    c->window_start = 0.0;
    c->last_fps = 0;
    c->last_window = 0.0;
}

void nq_fps_counter_tick(NqFpsCounter *c, double elapsed_seconds) {
    if (!c) return;
    /* window_start stays at 0.0 until the first window completes.
     * That way window on tick N is just elapsed_seconds (since the
     * start of the current window) — cleaner arithmetic than anchoring
     * window_start to the first tick's elapsed_seconds (which would
     * make window = (N-1) * dt instead of N * dt for N frames). */
    c->frame_count++;
    double window = elapsed_seconds - c->window_start;
    /* Trigger reset at >= 1.0s. Floor the window when computing FPS so
     * floating-point drift on exactly-1.0-second inputs (e.g. 60 frames
     * at 16.667ms each gives elapsed=1.000...001) doesn't round FPS down
     * to 59. The previous implementation suffered from this trap. */
    /* Trigger reset at window >= 0.999 (≈1.0 with 1ms tolerance).
     *
     * Why 0.999 and not 1.0:
     *   IEEE 754 multiplication drifts: 60 * (1.0/60.0) computes to
     *   0.9999999999999999 (not 1.0). With a strict >= 1.0 trigger
     *   the counter would never reset on perfect-rate timing.
     *   Real frame timing from clock_gettime has ±1ms drift too, so
     *   a 1ms tolerance matches physical reality.
     *
     * Why divisor = window (not (int)window):
     *   For windows in [0.999, 1.0) the (int) cast yields 0, causing
     *   division by zero. Using window directly gives the true average.
     */
    if (window >= 0.999) {
        c->last_fps = (int)(c->frame_count / window + 0.5);
        c->last_window = window;
        c->frame_count = 0;
        c->window_start = elapsed_seconds;
    }
}

int nq_fps_counter_get(const NqFpsCounter *c) {
    return c ? c->last_fps : 0;
}
