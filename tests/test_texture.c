/*
 * Texture module is mostly SDL-bound; in pure-C unit tests we exercise
 * the dimensions/query API via a stub path. The full load/draw flow is
 * exercised by CI's SDL3 build matrix.
 */
#include <nq/texture.h>
#include "test_main.c"

static void test_texture_init_zeroes(void) {
    NqTexture *t = NULL;
    NQ_ASSERT_EQ(nq_texture_width(t), 0);
    NQ_ASSERT_EQ(nq_texture_height(t), 0);
    /* safe to call destroy on NULL — should not crash */
    nq_texture_destroy(t);
    /* load with NULL args — defensive paths */
    NQ_ASSERT_EQ(nq_texture_load(NULL, "x"), NULL);
    /* safe to call draw/destroy on NULL */
    NQ_ASSERT(nq_texture_draw(NULL, 0, 0) == -1);
    NQ_ASSERT(nq_texture_draw_region(NULL, 0, 0, 0, 0, 1, 1) == -1);
}

static void test_load_mem_returns_null(void) {
    NqTexture *t = nq_texture_load_mem(NULL, "x", 1);
    NQ_ASSERT(t == NULL);
}

NQ_TEST_REGISTER("texture_init_zeroes",       test_texture_init_zeroes);
NQ_TEST_REGISTER("texture_load_mem_null",     test_load_mem_returns_null);
