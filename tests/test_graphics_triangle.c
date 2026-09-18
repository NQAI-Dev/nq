/* nq — triangle primitive tests.
 *
 * Covers the NULL-guard paths for draw_triangle and fill_triangle which
 * are exercisable without an SDL3 renderer (pass renderer=NULL → returns -1).
 *
 * The scanline geometry and actual pixel output are validated by the
 * examples/animated_square demo running in CI with a real SDL3 renderer.
 *
 * Pure C, no SDL3 required at test-runner level.
 */

#include "nq/graphics.h"
#include "test_main.c"

/* ------------------------------------------------------------------ */
/* draw_triangle                                                       */
/* ------------------------------------------------------------------ */

static void test_draw_triangle_null_renderer(void) {
    /* NULL renderer → -1 for any vertex combination. */
    NQ_ASSERT_EQ(nq_renderer_draw_triangle(NULL, NQ_COLOR_RGB(255,0,0),
                                            0, 0, 10, 0, 5, 10), -1);
}

static void test_draw_triangle_null_degenerate(void) {
    /* Even degenerate (all same point) returns -1 with NULL renderer. */
    NQ_ASSERT_EQ(nq_renderer_draw_triangle(NULL, NQ_COLOR_RGB(0,255,0),
                                            5, 5, 5, 5, 5, 5), -1);
}

/* ------------------------------------------------------------------ */
/* fill_triangle                                                       */
/* ------------------------------------------------------------------ */

static void test_fill_triangle_null_renderer(void) {
    /* NULL renderer → -1 regardless of vertex order. */
    NQ_ASSERT_EQ(nq_renderer_fill_triangle(NULL, NQ_COLOR_RGB(0,0,255),
                                            0, 0, 10, 0, 5, 10), -1);
}

static void test_fill_triangle_null_reversed_winding(void) {
    /* Clockwise winding also returns -1 with NULL renderer. */
    NQ_ASSERT_EQ(nq_renderer_fill_triangle(NULL, NQ_COLOR_RGB(128,128,128),
                                            5, 10, 10, 0, 0, 0), -1);
}

static void test_fill_triangle_null_flat_bottom(void) {
    /* Flat-bottom degenerate (y0==y1). */
    NQ_ASSERT_EQ(nq_renderer_fill_triangle(NULL, NQ_COLOR_RGB(0,0,0),
                                            0, 0, 10, 0, 5, 5), -1);
}

static void test_fill_triangle_null_flat_top(void) {
    /* Flat-top degenerate (y1==y2 after sort). */
    NQ_ASSERT_EQ(nq_renderer_fill_triangle(NULL, NQ_COLOR_RGB(0,0,0),
                                            5, 0, 0, 5, 10, 5), -1);
}

static void test_fill_triangle_null_collinear(void) {
    /* All three points collinear (horizontal line). */
    NQ_ASSERT_EQ(nq_renderer_fill_triangle(NULL, NQ_COLOR_RGB(0,0,0),
                                            0, 5, 5, 5, 10, 5), -1);
}

/* ------------------------------------------------------------------ */
/* Registration                                                        */
/* ------------------------------------------------------------------ */

NQ_TEST_REGISTER("draw_triangle/null_renderer",  test_draw_triangle_null_renderer)
NQ_TEST_REGISTER("draw_triangle/null_degenerate",test_draw_triangle_null_degenerate)
NQ_TEST_REGISTER("fill_triangle/null_renderer",  test_fill_triangle_null_renderer)
NQ_TEST_REGISTER("fill_triangle/null_reversed",  test_fill_triangle_null_reversed_winding)
NQ_TEST_REGISTER("fill_triangle/null_flat_bottom",test_fill_triangle_null_flat_bottom)
NQ_TEST_REGISTER("fill_triangle/null_flat_top",  test_fill_triangle_null_flat_top)
NQ_TEST_REGISTER("fill_triangle/null_collinear", test_fill_triangle_null_collinear)
