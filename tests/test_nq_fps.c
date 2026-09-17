/* nq — FPS counter tests.
 *
 * Verifies the rolling 1-second window behaves as advertised:
 *   - returns 0 before the first full window
 *   - returns frame_count / window_duration for the first window
 *   - resets correctly between consecutive windows
 *
 * Pure C, no SDL3. Compiles standalone with gcc.
 */

#include "nq/fps.h"

#include <assert.h>
#include <stdio.h>

static int g_failed = 0;
#define CHECK(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        g_failed++; \
    } \
} while (0)

static void test_get_before_first_window(void) {
    NqFpsCounter c;
    nq_fps_counter_init(&c);

    /* Tick 30 frames at 16.67ms each → 0.5s window, NOT yet 1s. */
    for (int i = 1; i <= 30; i++) {
        nq_fps_counter_tick(&c, i * (1.0 / 60.0));
    }
    /* Less than 1 second has elapsed; get() returns 0. */
    CHECK(nq_fps_counter_get(&c) == 0);
}

static void test_60fps_window(void) {
    NqFpsCounter c;
    nq_fps_counter_init(&c);

    /* 60 frames at 16.67ms each → exactly 1.0s window. */
    for (int i = 1; i <= 60; i++) {
        nq_fps_counter_tick(&c, i * (1.0 / 60.0));
    }
    CHECK(nq_fps_counter_get(&c) == 60);
}

static void test_30fps_window(void) {
    NqFpsCounter c;
    nq_fps_counter_init(&c);

    /* 30 frames at 33.33ms each → exactly 1.0s window → 30 FPS. */
    for (int i = 1; i <= 30; i++) {
        nq_fps_counter_tick(&c, i * (1.0 / 30.0));
    }
    CHECK(nq_fps_counter_get(&c) == 30);
}

static void test_reset_between_windows(void) {
    NqFpsCounter c;
    nq_fps_counter_init(&c);

    /* First window: 60 frames at 60fps. */
    for (int i = 1; i <= 60; i++) {
        nq_fps_counter_tick(&c, i * (1.0 / 60.0));
    }
    CHECK(nq_fps_counter_get(&c) == 60);

    /* Second window: 120 frames at 60fps over 2 seconds → 60 each. */
    for (int i = 1; i <= 120; i++) {
        nq_fps_counter_tick(&c, 1.0 + i * (1.0 / 60.0));
    }
    CHECK(nq_fps_counter_get(&c) == 60);

    /* Third window: 30 frames at 30fps. */
    for (int i = 1; i <= 30; i++) {
        nq_fps_counter_tick(&c, 3.0 + i * (1.0 / 30.0));
    }
    CHECK(nq_fps_counter_get(&c) == 30);
}

static void test_null_safety(void) {
    nq_fps_counter_init(NULL);     /* no crash */
    nq_fps_counter_tick(NULL, 1.0);
    CHECK(nq_fps_counter_get(NULL) == 0);
}

static void test_init_resets_existing(void) {
    NqFpsCounter c;
    nq_fps_counter_init(&c);
    for (int i = 1; i <= 60; i++) {
        nq_fps_counter_tick(&c, i * (1.0 / 60.0));
    }
    CHECK(nq_fps_counter_get(&c) == 60);

    /* Re-init resets the rolling window. */
    nq_fps_counter_init(&c);
    CHECK(nq_fps_counter_get(&c) == 0);
    /* Next 30 frames haven't completed a 1s window yet. */
    for (int i = 1; i <= 30; i++) {
        nq_fps_counter_tick(&c, i * (1.0 / 60.0));
    }
    CHECK(nq_fps_counter_get(&c) == 0);
}

int main(void) {
    test_get_before_first_window();
    test_60fps_window();
    test_30fps_window();
    test_reset_between_windows();
    test_null_safety();
    test_init_resets_existing();
    if (g_failed) {
        fprintf(stderr, "%d failure(s)\n", g_failed);
        return 1;
    }
    printf("ok\n");
    return 0;
}
