/* nq — tests for mouse input helpers.
 *
 * Covers:
 *   - nq_input_mouse_wheel: setter accumulates; begin_frame resets
 *   - nq_input_mouse_inside_rect: hit-test incl. boundary / empty
 *
 * Pure C, no SDL3. Builds on the existing tests/test_nq_input.c pattern.
 */

#include "nq/input.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static int g_failed = 0;
#define CHECK(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        g_failed++; \
    } \
} while (0)

static void test_wheel_accumulates_then_resets(void) {
    NqInput in;
    nq_input_init(&in);

    /* Multiple wheel ticks in one frame accumulate. */
    nq_input_set_mouse_wheel(&in, 3);
    nq_input_set_mouse_wheel(&in, -2);
    nq_input_set_mouse_wheel(&in, 1);
    CHECK(nq_input_mouse_wheel(&in) == 2);

    /* begin_frame() resets wheel to 0 like rel_x/rel_y. */
    nq_input_begin_frame(&in);
    CHECK(nq_input_mouse_wheel(&in) == 0);

    /* After reset, fresh ticks accumulate again. */
    nq_input_set_mouse_wheel(&in, 5);
    CHECK(nq_input_mouse_wheel(&in) == 5);
}

static void test_inside_rect_hit_test(void) {
    NqInput in;
    nq_input_init(&in);
    /* (50, 50) hits "clearly inside" cases for rect (0,0,100,100) and
     * falls outside right-edge / below / left-of smaller rects. */
    nq_input_set_mouse_pos(&in, 50, 50);

    /* Inside big rect. */
    CHECK(nq_input_mouse_inside_rect(&in, 0, 0, 100, 100) == 1);
    /* Outside: pos.x (50) < rect.x (100). */
    CHECK(nq_input_mouse_inside_rect(&in, 100, 0, 50, 50) == 0);
    /* Outside: pos.y (50) < rect.y (100). */
    CHECK(nq_input_mouse_inside_rect(&in, 0, 100, 50, 50) == 0);
    /* Move pos to inclusive top-left corner of rect (0, 50, 50, 50):
     * x in [0, 50), y in [50, 100). pos (0, 50) is INSIDE. */
    nq_input_set_mouse_pos(&in, 0, 50);
    CHECK(nq_input_mouse_inside_rect(&in, 0, 50, 50, 50) == 1);
    /* Half-open: pos on exclusive right edge is OUTSIDE.
     * Move pos to (50, 50) which is the exclusive bottom-right corner
     * of rect (0, 50, 50, 50): x == x+w and y == y. OUTSIDE. */
    nq_input_set_mouse_pos(&in, 50, 50);
    CHECK(nq_input_mouse_inside_rect(&in, 0, 50, 50, 50) == 0);
    /* Half-open: pos on exclusive bottom edge of rect (0, 0, 50, 50)
     * is OUTSIDE. pos (25, 50) is y == y+h. */
    nq_input_set_mouse_pos(&in, 25, 50);
    CHECK(nq_input_mouse_inside_rect(&in, 0, 0, 50, 50) == 0);
    /* Empty rect never matches (w==0, h==0, negative w). */
    nq_input_set_mouse_pos(&in, 50, 50);
    CHECK(nq_input_mouse_inside_rect(&in, 50, 50, 0, 50) == 0);
    CHECK(nq_input_mouse_inside_rect(&in, 50, 50, 50, 0) == 0);
    CHECK(nq_input_mouse_inside_rect(&in, 50, 50, -10, 50) == 0);
}

static void test_null_safety(void) {
    CHECK(nq_input_mouse_wheel(NULL) == 0);
    CHECK(nq_input_mouse_inside_rect(NULL, 0, 0, 10, 10) == 0);
    nq_input_set_mouse_wheel(NULL, 5);  /* no crash */
}

int main(void) {
    test_wheel_accumulates_then_resets();
    test_inside_rect_hit_test();
    test_null_safety();
    if (g_failed) {
        fprintf(stderr, "%d failure(s)\n", g_failed);
        return 1;
    }
    printf("ok\n");
    return 0;
}
