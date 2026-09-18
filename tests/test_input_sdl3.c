#include <SDL3/SDL.h>
#include <nq/input_sdl3.h>
#include "test_main.c"

/* Tests need SDL_Init for SDL_GetScancodeFromKey etc. — but that needs
 * display/audio etc. which we can't init from a unit test. Use NULL
 * event paths instead: feed crafted SDL_Event structs directly. */
static SDL_Event make_key(SDL_EventType t, SDL_Keycode k) {
    SDL_Event e = {0};
    e.type = t;
    e.key.key = k;
    e.key.scancode = SDL_GetScancodeFromKey(k, NULL);  /* safe without init */
    return e;
}
static SDL_Event make_btn(SDL_EventType t, uint8_t button) {
    SDL_Event e = {0};
    e.type = t;
    e.button.button = button;
    return e;
}
static SDL_Event make_motion(int x, int y) {
    SDL_Event e = {0};
    e.type = SDL_EVENT_MOUSE_MOTION;
    e.motion.x = x;
    e.motion.y = y;
    return e;
}

static void test_key_down_up_pump(void) {
    NqInput in;
    nq_input_init(&in);
    nq_input_begin_frame(&in);

    SDL_Event down = make_key(SDL_EVENT_KEY_DOWN, SDLK_SPACE);
    SDL_Event up   = make_key(SDL_EVENT_KEY_UP,   SDLK_SPACE);
    NQ_ASSERT_EQ(nq_input_pump_sdl3_event(&in, &down), 1);
    NQ_ASSERT_EQ(nq_input_pump_sdl3_event(&in, &up),   1);
    /* After down: key down. After up: key up. */
    NQ_ASSERT_EQ(nq_input_key_down(&in, SDL_GetScancodeFromKey(SDLK_SPACE, NULL)), 1);
    /* Press → released edge in current frame: */
    nq_input_begin_frame(&in);
    NQ_ASSERT_EQ(nq_input_key_down(&in, SDL_GetScancodeFromKey(SDLK_SPACE, NULL)), 0);
    NQ_ASSERT_EQ(nq_input_key_released(&in, SDL_GetScancodeFromKey(SDLK_SPACE, NULL)), 1);
}

static void test_mouse_button_pump(void) {
    NqInput in;
    nq_input_init(&in);
    nq_input_begin_frame(&in);

    SDL_Event ld = make_btn(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT);
    SDL_Event lu = make_btn(SDL_EVENT_MOUSE_BUTTON_UP,   SDL_BUTTON_LEFT);
    SDL_Event rd = make_btn(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_RIGHT);
    NQ_ASSERT_EQ(nq_input_pump_sdl3_event(&in, &ld), 1);
    NQ_ASSERT_EQ(nq_input_key_down(&in, SDL_GetScancodeFromKey(SDLK_SPACE, NULL)), 0);  /* unchanged */
    NQ_ASSERT(nq_input_mouse_down(&in, NQ_MOUSE_BUTTON_LEFT));
    NQ_ASSERT(!nq_input_mouse_down(&in, NQ_MOUSE_BUTTON_RIGHT));
    NQ_ASSERT_EQ(nq_input_pump_sdl3_event(&in, &rd), 1);
    NQ_ASSERT(nq_input_mouse_down(&in, NQ_MOUSE_BUTTON_LEFT));
    NQ_ASSERT(nq_input_mouse_down(&in, NQ_MOUSE_BUTTON_RIGHT));
    NQ_ASSERT_EQ(nq_input_pump_sdl3_event(&in, &lu), 1);
    NQ_ASSERT(!nq_input_mouse_down(&in, NQ_MOUSE_BUTTON_LEFT));
    NQ_ASSERT(nq_input_mouse_down(&in, NQ_MOUSE_BUTTON_RIGHT));
}

static void test_mouse_motion_pump(void) {
    NqInput in;
    nq_input_init(&in);
    nq_input_begin_frame(&in);

    SDL_Event m1 = make_motion(100, 50);
    SDL_Event m2 = make_motion(150, 80);
    NQ_ASSERT_EQ(nq_input_pump_sdl3_event(&in, &m1), 1);
    NQ_ASSERT_EQ(nq_input_pump_sdl3_event(&in, &m2), 1);
    NQ_ASSERT_EQ(nq_input_mouse_x(&in), 150);
    NQ_ASSERT_EQ(nq_input_mouse_y(&in), 80);
}

static void test_unknown_event_returns_zero(void) {
    NqInput in;
    nq_input_init(&in);
    SDL_Event e = {0};
    e.type = SDL_EVENT_QUIT;  /* quit isn't an input event */
    NQ_ASSERT_EQ(nq_input_pump_sdl3_event(&in, &e), 0);
}

static void test_null_safe(void) {
    SDL_Event e = make_key(SDL_EVENT_KEY_DOWN, SDLK_SPACE);
    NQ_ASSERT_EQ(nq_input_pump_sdl3_event(NULL, &e), 0);
    NQ_ASSERT_EQ(nq_input_pump_sdl3_event(NULL, NULL), 0);
    NqInput in;
    nq_input_init(&in);
    NQ_ASSERT_EQ(nq_input_pump_sdl3_event(&in, NULL), 0);
}

NQ_TEST_REGISTER("input_sdl3_key_down_up",    test_key_down_up_pump);
NQ_TEST_REGISTER("input_sdl3_mouse_button",  test_mouse_button_pump);
NQ_TEST_REGISTER("input_sdl3_mouse_motion",  test_mouse_motion_pump);
NQ_TEST_REGISTER("input_sdl3_unknown_event", test_unknown_event_returns_zero);
NQ_TEST_REGISTER("input_sdl3_null_safe",     test_null_safe);
