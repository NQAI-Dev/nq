/* Tests for nq_input_clear. The base test_input.c covers the normal
 * setters / queries; this file focuses on the reset-all path. */
#include <nq/input.h>
#include "test_main.c"

static void test_clear_zeros_keys(void) {
    NqInput in;
    nq_input_init(&in);
    nq_input_set_key(&in, 42, 1);
    nq_input_set_key(&in, 7, 1);
    NQ_ASSERT(nq_input_key_down(&in, 42));
    nq_input_clear(&in);
    NQ_ASSERT_EQ(nq_input_key_down(&in, 42), 0);
    NQ_ASSERT_EQ(nq_input_key_down(&in, 7), 0);
}

static void test_clear_zeros_mouse_buttons(void) {
    NqInput in;
    nq_input_init(&in);
    nq_input_set_mouse_button(&in, NQ_MOUSE_BUTTON_LEFT | NQ_MOUSE_BUTTON_RIGHT);
    NQ_ASSERT(nq_input_mouse_down(&in, NQ_MOUSE_BUTTON_LEFT));
    nq_input_clear(&in);
    NQ_ASSERT_EQ(nq_input_mouse_down(&in, NQ_MOUSE_BUTTON_LEFT), 0);
    NQ_ASSERT_EQ(nq_input_mouse_down(&in, NQ_MOUSE_BUTTON_RIGHT), 0);
}

static void test_clear_zeros_mouse_position(void) {
    NqInput in;
    nq_input_init(&in);
    nq_input_set_mouse_pos(&in, 200, 150);
    nq_input_begin_frame(&in);
    /* Add another move to accumulate deltas. */
    nq_input_set_mouse_pos(&in, 250, 170);
    NQ_ASSERT_EQ(nq_input_mouse_x(&in), 250);
    NQ_ASSERT_EQ(nq_input_mouse_y(&in), 170);
    NQ_ASSERT_EQ(nq_input_mouse_dx(&in), 50);
    nq_input_clear(&in);
    NQ_ASSERT_EQ(nq_input_mouse_x(&in), 0);
    NQ_ASSERT_EQ(nq_input_mouse_y(&in), 0);
    NQ_ASSERT_EQ(nq_input_mouse_dx(&in), 0);
    NQ_ASSERT_EQ(nq_input_mouse_dy(&in), 0);
}

static void test_clear_makes_pressed_safe(void) {
    /* After clear, begin_frame must NOT fire pressed for keys that were
     * down before clear (current == prev == 0 after clear). */
    NqInput in;
    nq_input_init(&in);
    nq_input_set_key(&in, 100, 1);
    nq_input_begin_frame(&in);  /* prev now has key 100 down */
    NQ_ASSERT_EQ(nq_input_key_pressed(&in, 100), 0);  /* current == prev */
    nq_input_clear(&in);
    nq_input_begin_frame(&in);
    /* Now both current and prev are 0 → no pressed edge. */
    NQ_ASSERT_EQ(nq_input_key_pressed(&in, 100), 0);
    NQ_ASSERT_EQ(nq_input_key_released(&in, 100), 0);
}

static void test_clear_null_safe(void) {
    nq_input_clear(NULL);
    /* Should not crash. Reaching here means success. */
    NQ_ASSERT(1);
}

NQ_TEST_REGISTER("input_clear_zeros_keys",            test_clear_zeros_keys)
NQ_TEST_REGISTER("input_clear_zeros_mouse_buttons",   test_clear_zeros_mouse_buttons)
NQ_TEST_REGISTER("input_clear_zeros_mouse_position",  test_clear_zeros_mouse_position)
NQ_TEST_REGISTER("input_clear_makes_pressed_safe",    test_clear_makes_pressed_safe)
NQ_TEST_REGISTER("input_clear_null_safe",              test_clear_null_safe)
