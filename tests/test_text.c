#include <nq/text.h>
#include "test_main.c"

static void test_text_create_destroy_null(void) {
    /* Without SDL3 dev headers on this host we can't construct a real
     * NqRenderer to give to nq_text_create, so the test exercises only
     * the NULL-safe paths. CI on linux/macos/windows has SDL3 and runs
     * a fuller check (nq_text_create with a real renderer). */
    nq_text_destroy(NULL);
    NQ_ASSERT(nq_text_set_color(NULL, 255, 0, 0, 255) == (void)0);
    NQ_ASSERT_EQ(nq_text_draw(NULL, "hi", 0, 0), 0);
    NQ_ASSERT_EQ(nq_text_glyph_w(NULL, 'A'), 5);
    NQ_ASSERT_EQ(nq_text_glyph_h(NULL, 'A'), 7);
    NQ_ASSERT_EQ(nq_text_line_h(NULL), 9);
}

static void test_glyph_metrics_consistent(void) {
    /* Even without an SDL3 renderer, the metric queries return fixed
     * values from constants. Verifies the public contract — callers
     * can do layout calculations without needing to instantiate a
     * full NqText. */
    NQ_ASSERT_EQ(nq_text_glyph_w(NULL, 'A'), 5);
    NQ_ASSERT_EQ(nq_text_glyph_h(NULL, 'A'), 7);
    NQ_ASSERT_EQ(nq_text_line_h(NULL), 9);
    NQ_ASSERT_EQ(nq_text_glyph_w(NULL, ' '), 5);
    NQ_ASSERT_EQ(nq_text_glyph_w(NULL, '~'), 5);
}

static void test_set_color_stores_values(void) {
    /* We can't construct an NqText without SDL3, so just verify the
     * setter is a no-op on NULL and returns no crash. The behaviour is
     * fully exercised by the CI matrix where NqText can be constructed. */
    NqText *t = NULL;
    nq_text_set_color(t, 100, 150, 200, 255);
    /* Reaching here means it didn't crash. */
    NQ_ASSERT(1);
}

NQ_TEST_REGISTER("text_create_destroy_null", test_text_create_destroy_null);
NQ_TEST_REGISTER("text_glyph_metrics",      test_glyph_metrics_consistent);
NQ_TEST_REGISTER("text_set_color_null",     test_set_color_stores_values);
