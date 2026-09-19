#include <nq/input.h>
#include "test_main.c"

static void test_input_init_zeroes(void) {
    NqInput in;
    memset(&in, 0xAA, sizeof(in));  /* poison */
    nq_input_init(&in);
    NQ_ASSERT_EQ(in.kb.current[0], 0);
    NQ_ASSERT_EQ(in.kb.prev[0], 0);
    NQ_ASSERT_EQ(in.mouse.x, 0);
    NQ_ASSERT_EQ(in.mouse.y, 0);
    NQ_ASSERT_EQ(in.mouse.buttons, 0);
    NQ_ASSERT_EQ(in.mouse.rel_x, 0);
    NQ_ASSERT_EQ(in.mouse.rel_y, 0);
}

static void test_key_down_pressed_released(void) {
    NqInput in;
    nq_input_init(&in);
    /* Frame 0: press scancode 42 */
    nq_input_set_key(&in, 42, 1);
    nq_input_begin_frame(&in);  /* snapshot current -> prev */
    /* Frame 1 begins. Scancode 42 is currently held; pressed = 0 (was already down in prev too) */
    NQ_ASSERT_EQ(nq_input_key_down(&in, 42), 1);
    NQ_ASSERT_EQ(nq_input_key_pressed(&in, 42), 0);
    NQ_ASSERT_EQ(nq_input_key_released(&in, 42), 0);

    /* Frame 1: release scancode 42 */
    nq_input_begin_frame(&in);  /* snapshot held state before events */
    nq_input_set_key(&in, 42, 0);
    /* Released during this frame. */
    NQ_ASSERT_EQ(nq_input_key_down(&in, 42), 0);
    NQ_ASSERT_EQ(nq_input_key_pressed(&in, 42), 0);
    NQ_ASSERT_EQ(nq_input_key_released(&in, 42), 1);
}

static void test_key_press_edge_only_once(void) {
    NqInput in;
    nq_input_init(&in);
    /* Scancode 7: down -> begin -> check pressed == 1 (was 0 in prev) */
    nq_input_set_key(&in, 7, 1);
    NQ_ASSERT_EQ(nq_input_key_pressed(&in, 7), 1);
    nq_input_begin_frame(&in);
    /* Now it's in prev. Continued pressed this frame — pressed == 0 */
    NQ_ASSERT_EQ(nq_input_key_pressed(&in, 7), 0);
    NQ_ASSERT_EQ(nq_input_key_down(&in, 7), 1);
}

static void test_out_of_range_scancode_safe(void) {
    NqInput in;
    nq_input_init(&in);
    /* Should not crash; returns 0 / no-op. */
    nq_input_set_key(&in, -1, 1);
    nq_input_set_key(&in, NQ_KEY_MAX, 1);
    NQ_ASSERT_EQ(nq_input_key_down(&in, -1), 0);
    NQ_ASSERT_EQ(nq_input_key_down(&in, NQ_KEY_MAX), 0);
    nq_input_begin_frame(&in);
    nq_input_set_key(&in, -1, 0);
}

static void test_mouse_position_and_delta(void) {
    NqInput in;
    nq_input_init(&in);
    /* Pump a 5,10 mouse move */
    nq_input_set_mouse_pos(&in, 5, 10);
    NQ_ASSERT_EQ(nq_input_mouse_x(&in), 5);
    NQ_ASSERT_EQ(nq_input_mouse_y(&in), 10);
    nq_input_begin_frame(&in);  /* resets rel_x/rel_y to 0 */
    NQ_ASSERT_EQ(nq_input_mouse_dx(&in), 0);
    NQ_ASSERT_EQ(nq_input_mouse_dy(&in), 0);

    /* Pump a 8,15 move — rel_x = 8-5 = 3 */
    nq_input_set_mouse_pos(&in, 8, 15);
    NQ_ASSERT_EQ(nq_input_mouse_x(&in), 8);
    NQ_ASSERT_EQ(nq_input_mouse_y(&in), 15);
    NQ_ASSERT_EQ(nq_input_mouse_dx(&in), 3);
    NQ_ASSERT_EQ(nq_input_mouse_dy(&in), 5);
    nq_input_begin_frame(&in);

    /* After begin_frame, dx/dy reset but x/y keep current */
    NQ_ASSERT_EQ(nq_input_mouse_dx(&in), 0);
    NQ_ASSERT_EQ(nq_input_mouse_dy(&in), 0);
    NQ_ASSERT_EQ(nq_input_mouse_x(&in), 8);
}

static void test_mouse_button_down_pressed(void) {
    NqInput in;
    nq_input_init(&in);
    /* Press left button */
    nq_input_set_mouse_button(&in, NQ_MOUSE_BUTTON_LEFT);
    NQ_ASSERT_EQ(nq_input_mouse_down(&in, NQ_MOUSE_BUTTON_LEFT), 1);
    NQ_ASSERT_EQ(nq_input_mouse_pressed(&in, NQ_MOUSE_BUTTON_LEFT), 1);
    nq_input_begin_frame(&in);
    /* Held across frames — down yes, pressed no */
    NQ_ASSERT_EQ(nq_input_mouse_down(&in, NQ_MOUSE_BUTTON_LEFT), 1);
    NQ_ASSERT_EQ(nq_input_mouse_pressed(&in, NQ_MOUSE_BUTTON_LEFT), 0);
    /* Release */
    nq_input_set_mouse_button(&in, 0);
    nq_input_begin_frame(&in);
    NQ_ASSERT_EQ(nq_input_mouse_down(&in, NQ_MOUSE_BUTTON_LEFT), 0);
    NQ_ASSERT_EQ(nq_input_mouse_pressed(&in, NQ_MOUSE_BUTTON_LEFT), 0);
}

static void test_mouse_button_mask_combo(void) {
    NqInput in;
    nq_input_init(&in);
    /* Both left and right down */
    nq_input_set_mouse_button(&in, NQ_MOUSE_BUTTON_LEFT | NQ_MOUSE_BUTTON_RIGHT);
    NQ_ASSERT_EQ(nq_input_mouse_down(&in, NQ_MOUSE_BUTTON_LEFT), 1);
    NQ_ASSERT_EQ(nq_input_mouse_down(&in, NQ_MOUSE_BUTTON_RIGHT), 1);
    /* Querying for both at once — both down */
    NQ_ASSERT_EQ(nq_input_mouse_down(&in, NQ_MOUSE_BUTTON_LEFT | NQ_MOUSE_BUTTON_RIGHT), 1);
    nq_input_begin_frame(&in);
    /* Edge detection per-button: pressed flags should NOT clear across begin_frame
     * until they're actually consumed — by convention we leave them set; caller
     * queries once per frame. */
    nq_input_set_mouse_button(&in, NQ_MOUSE_BUTTON_LEFT | NQ_MOUSE_BUTTON_RIGHT);  /* still both held */
    NQ_ASSERT_EQ(nq_input_mouse_pressed(&in, NQ_MOUSE_BUTTON_LEFT), 0);
    NQ_ASSERT_EQ(nq_input_mouse_pressed(&in, NQ_MOUSE_BUTTON_RIGHT), 0);
}

NQ_TEST_REGISTER("input_init_zeroes",              test_input_init_zeroes)
NQ_TEST_REGISTER("key_down_pressed_released",     test_key_down_pressed_released)
NQ_TEST_REGISTER("key_press_edge_only_once",       test_key_press_edge_only_once)
NQ_TEST_REGISTER("out_of_range_scancode_safe",     test_out_of_range_scancode_safe)
NQ_TEST_REGISTER("mouse_position_and_delta",       test_mouse_position_and_delta)
NQ_TEST_REGISTER("mouse_button_down_pressed",     test_mouse_button_down_pressed)
NQ_TEST_REGISTER("mouse_button_mask_combo",       test_mouse_button_mask_combo)
