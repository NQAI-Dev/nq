#include <nq/color.h>
#include "test_main.c"

static void test_lerp_endpoints(void) {
    NqColor red   = NQ_COLOR_RGB(255, 0, 0);
    NqColor blue  = NQ_COLOR_RGB(0, 0, 255);
    NqColor at_0  = nq_color_lerp(red, blue, 0.0f);
    NqColor at_1  = nq_color_lerp(red, blue, 1.0f);
    NQ_ASSERT_EQ(at_0.r, 255);
    NQ_ASSERT_EQ(at_0.b, 0);
    NQ_ASSERT_EQ(at_1.r, 0);
    NQ_ASSERT_EQ(at_1.b, 255);
}

static void test_lerp_midpoint(void) {
    NqColor black = NQ_COLOR_RGB(0, 0, 0);
    NqColor white = NQ_COLOR_RGB(255, 255, 255);
    NqColor mid   = nq_color_lerp(black, white, 0.5f);
    /* Midpoint is 127 or 128 depending on rounding. */
    NQ_ASSERT(mid.r >= 127 && mid.r <= 128);
    NQ_ASSERT(mid.g >= 127 && mid.g <= 128);
    NQ_ASSERT(mid.b >= 127 && mid.b <= 128);
}

static void test_lerp_alpha(void) {
    NqColor a = NQ_COLOR_RGBA(0, 0, 0, 100);
    NqColor b = NQ_COLOR_RGBA(0, 0, 0, 200);
    NqColor mid = nq_color_lerp(a, b, 0.5f);
    NQ_ASSERT(mid.a >= 149 && mid.a <= 150);
}

static void test_lerp_clamps_t(void) {
    NqColor red  = NQ_COLOR_RGB(255, 0, 0);
    NqColor blue = NQ_COLOR_RGB(0, 0, 255);
    /* t < 0 → clamps to 0 (returns a) */
    NqColor under = nq_color_lerp(red, blue, -0.5f);
    NQ_ASSERT_EQ(under.r, 255);
    NQ_ASSERT_EQ(under.b, 0);
    /* t > 1 → clamps to 1 (returns b) */
    NqColor over = nq_color_lerp(red, blue, 1.5f);
    NQ_ASSERT_EQ(over.r, 0);
    NQ_ASSERT_EQ(over.b, 255);
}

static void test_lerp_identity(void) {
    /* lerp(x, x, t) == x for any t (after clamp). */
    NqColor c = NQ_COLOR_RGB(50, 100, 200);
    NQ_ASSERT_EQ(nq_color_lerp(c, c, 0.0f).r, 50);
    NQ_ASSERT_EQ(nq_color_lerp(c, c, 0.7f).r, 50);
    NQ_ASSERT_EQ(nq_color_lerp(c, c, 1.0f).r, 50);
}

static void test_from_uint32(void) {
    NqColor c = nq_color_from_uint32(0xFF8040C8u);
    NQ_ASSERT_EQ(c.r, 0xFF);
    NQ_ASSERT_EQ(c.g, 0x80);
    NQ_ASSERT_EQ(c.b, 0x40);
    NQ_ASSERT_EQ(c.a, 0xC8);
    /* Zero input — fully transparent black. */
    NqColor z = nq_color_from_uint32(0u);
    NQ_ASSERT_EQ(z.r, 0);
    NQ_ASSERT_EQ(z.a, 0);
}

NQ_TEST_REGISTER("color_lerp_endpoints",          test_lerp_endpoints);
NQ_TEST_REGISTER("color_lerp_midpoint",           test_lerp_midpoint);
NQ_TEST_REGISTER("color_lerp_alpha",              test_lerp_alpha);
NQ_TEST_REGISTER("color_lerp_clamps_t",           test_lerp_clamps_t);
NQ_TEST_REGISTER("color_lerp_identity",           test_lerp_identity);
NQ_TEST_REGISTER("color_from_uint32",             test_from_uint32);
