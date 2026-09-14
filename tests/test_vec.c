#include <nq/vec.h>
#include "test_main.c"  /* for NQ_TEST_REGISTER / NQ_ASSERT_EQ */

static void test_vec2i_create(void) {
    NqVec2i v = nq_vec2i(3, -7);
    NQ_ASSERT_EQ(v.x, 3);
    NQ_ASSERT_EQ(v.y, -7);
}

static void test_vec2i_add(void) {
    NqVec2i a = nq_vec2i(3, 4);
    NqVec2i b = nq_vec2i(10, -2);
    NqVec2i c = nq_vec2i_add(a, b);
    NQ_ASSERT_EQ(c.x, 13);
    NQ_ASSERT_EQ(c.y, 2);
}

static void test_vec2i_sub(void) {
    NqVec2i a = nq_vec2i(10, 5);
    NqVec2i b = nq_vec2i(3, 8);
    NqVec2i c = nq_vec2i_sub(a, b);
    NQ_ASSERT_EQ(c.x, 7);
    NQ_ASSERT_EQ(c.y, -3);
}

static void test_vec2i_scale(void) {
    NqVec2i v = nq_vec2i_scale(nq_vec2i(2, -3), 4);
    NQ_ASSERT_EQ(v.x, 8);
    NQ_ASSERT_EQ(v.y, -12);
}

static void test_vec2i_eq(void) {
    NQ_ASSERT(nq_vec2i_eq(nq_vec2i(1, 2), nq_vec2i(1, 2)));
    NQ_ASSERT(!nq_vec2i_eq(nq_vec2i(1, 2), nq_vec2i(2, 1)));
    NQ_ASSERT(!nq_vec2i_eq(nq_vec2i(0, 0), nq_vec2i(1, 0)));
}

static void test_vec2f_add(void) {
    NqVec2f a = nq_vec2f(1.5f, 2.5f);
    NqVec2f b = nq_vec2f(0.5f, -0.5f);
    NqVec2f c = nq_vec2f_add(a, b);
    NQ_ASSERT(c.x > 1.99f && c.x < 2.01f);
    NQ_ASSERT(c.y > 1.99f && c.y < 2.01f);
}

NQ_TEST_REGISTER("vec2i_create",     test_vec2i_create);
NQ_TEST_REGISTER("vec2i_add",        test_vec2i_add);
NQ_TEST_REGISTER("vec2i_sub",        test_vec2i_sub);
NQ_TEST_REGISTER("vec2i_scale",      test_vec2i_scale);
NQ_TEST_REGISTER("vec2i_eq",         test_vec2i_eq);
NQ_TEST_REGISTER("vec2f_add",        test_vec2f_add);
