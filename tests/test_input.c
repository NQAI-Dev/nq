#include <nq/input.h>
#include "test_main.c"  /* NQ_TEST_REGISTER / NQ_ASSERT / NQ_ASSERT_EQ */

static void press(NqInput *in, NqKey k) {
    NqEvent ev = { NQ_EVENT_KEY_DOWN, k };
    nq_input_process(in, ev);
}

static void release(NqInput *in, NqKey k) {
    NqEvent ev = { NQ_EVENT_KEY_UP, k };
    nq_input_process(in, ev);
}

static void test_basic_hold_and_release(void) {
    NqInput *in = nq_input_create();
    nq_input_poll_begin(in);

    /* Press SPACE: within the same frame, _down and _pressed both true. */
    press(in, NQ_KEY_SPACE);
    NQ_ASSERT(nq_input_down(in, NQ_KEY_SPACE));
    NQ_ASSERT(nq_input_pressed(in, NQ_KEY_SPACE));
    NQ_ASSERT(!nq_input_released(in, NQ_KEY_SPACE));

    /* Release SPACE: _down false, _released true, _pressed false. */
    release(in, NQ_KEY_SPACE);
    NQ_ASSERT(!nq_input_down(in, NQ_KEY_SPACE));
    NQ_ASSERT(!nq_input_pressed(in, NQ_KEY_SPACE));
    NQ_ASSERT(nq_input_released(in, NQ_KEY_SPACE));

    nq_input_destroy(in);
}

static void test_poll_begin_clears_edges(void) {
    NqInput *in = nq_input_create();

    /* Frame 1: press SPACE, release at end. */
    press(in, NQ_KEY_SPACE);
    release(in, NQ_KEY_SPACE);
    /* Both edges set within frame 1. */
    nq_input_poll_begin(in);
    /* After poll_begin: edges cleared but _down state is preserved. */
    NQ_ASSERT(!nq_input_pressed(in, NQ_KEY_SPACE));
    NQ_ASSERT(!nq_input_released(in, NQ_KEY_SPACE));
    /* SPACE is no longer held (released at end of frame 1). */
    NQ_ASSERT(!nq_input_down(in, NQ_KEY_SPACE));

    /* Frame 2: still in "after poll_begin" state — query down=false,
     * edges=false. Now press again. */
    press(in, NQ_KEY_A);
    NQ_ASSERT(nq_input_pressed(in, NQ_KEY_A));
    NQ_ASSERT(nq_input_down(in, NQ_KEY_A));

    nq_input_destroy(in);
}

static void test_independent_keys(void) {
    NqInput *in = nq_input_create();
    nq_input_poll_begin(in);

    press(in, NQ_KEY_A);
    press(in, NQ_KEY_LEFT);
    NQ_ASSERT(nq_input_down(in, NQ_KEY_A));
    NQ_ASSERT(nq_input_down(in, NQ_KEY_LEFT));
    NQ_ASSERT(!nq_input_down(in, NQ_KEY_B));
    NQ_ASSERT(!nq_input_down(in, NQ_KEY_RIGHT));

    release(in, NQ_KEY_A);
    NQ_ASSERT(!nq_input_down(in, NQ_KEY_A));
    NQ_ASSERT(nq_input_down(in, NQ_KEY_LEFT));
    NQ_ASSERT(nq_input_released(in, NQ_KEY_A));
    NQ_ASSERT(!nq_input_released(in, NQ_KEY_LEFT));

    nq_input_destroy(in);
}

static void test_quit_flag(void) {
    NqInput *in = nq_input_create();

    NqEvent quit = { NQ_EVENT_QUIT, NQ_KEY_NONE };
    nq_input_process(in, quit);
    NQ_ASSERT(nq_input_quit(in));

    nq_input_poll_begin(in);
    NQ_ASSERT(!nq_input_quit(in));  /* cleared with the rest of edges */

    nq_input_destroy(in);
}

static void test_destroy_null_safety(void) {
    /* Passing NULL to destroy / poll_begin / queries / process must not
     * crash. SDL bridge functions should accept any int. */
    nq_input_destroy(NULL);
    nq_input_poll_begin(NULL);
    NQ_ASSERT(!nq_input_pressed (NULL, NQ_KEY_A));
    NQ_ASSERT(!nq_input_released(NULL, NQ_KEY_A));
    NQ_ASSERT(!nq_input_down    (NULL, NQ_KEY_A));
    NQ_ASSERT(!nq_input_quit    (NULL));
    nq_input_process(NULL, (NqEvent){ NQ_EVENT_KEY_DOWN, NQ_KEY_A });
    /* SDL bridge: unknown scancode yields NQ_KEY_NONE event. */
    NqEvent ev = nq_input_make_keydown_event(0xDEAD);
    NQ_ASSERT(ev.kind == NQ_EVENT_KEY_DOWN);
    NQ_ASSERT(ev.key  == NQ_KEY_NONE);
}

static void test_unknown_key_ignored(void) {
    NqInput *in = nq_input_create();

    /* NQ_KEY_NONE on DOWN/UP must not flip state. */
    NqEvent down = { NQ_EVENT_KEY_DOWN, NQ_KEY_NONE };
    NqEvent up   = { NQ_EVENT_KEY_UP,   NQ_KEY_NONE };
    nq_input_poll_begin(in);
    nq_input_process(in, down);
    nq_input_process(in, up);
    /* Nothing should be set; queries for a random in-range key must be false. */
    NQ_ASSERT(!nq_input_pressed (in, NQ_KEY_A));
    NQ_ASSERT(!nq_input_released(in, NQ_KEY_A));
    NQ_ASSERT(!nq_input_down    (in, NQ_KEY_A));
}

NQ_TEST_REGISTER("input_basic_hold_release", test_basic_hold_and_release);
NQ_TEST_REGISTER("input_poll_begin_clears_edges", test_poll_begin_clears_edges);
NQ_TEST_REGISTER("input_independent_keys", test_independent_keys);
NQ_TEST_REGISTER("input_quit_flag", test_quit_flag);
NQ_TEST_REGISTER("input_destroy_null_safety", test_destroy_null_safety);
NQ_TEST_REGISTER("input_unknown_key_ignored", test_unknown_key_ignored);
