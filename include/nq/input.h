/*
 * nq — keyboard state machine.
 *
 * Pure C, no SDL dependency on the data side — SDL3 lives behind the
 * bridge functions (nq_input_make_keydown_event / nq_input_make_keyup_event)
 * so tests can drive the state machine without SDL3 dev headers.
 *
 * Lifecycle per frame:
 *   1. nq_input_poll_begin(in)   — clear edges from previous frame
 *   2. for each event: nq_input_process(in, ev)
 *   3. queries (pressed/released/down) return correct values
 *
 * Edges stick for one frame: a key DOWN registers true from _pressed()
 * once at the frame it arrived, and continues to register from _down()
 * until _released_ fires.
 */
#ifndef NQ_INPUT_H
#define NQ_INPUT_H

#include <stdbool.h>
#include <stdint.h>

#define NQ_KEY_COUNT 256

typedef enum {
    NQ_KEY_NONE = 0,
    NQ_KEY_A, NQ_KEY_B, NQ_KEY_C, NQ_KEY_D, NQ_KEY_E, NQ_KEY_F,
    NQ_KEY_G, NQ_KEY_H, NQ_KEY_I, NQ_KEY_J, NQ_KEY_K, NQ_KEY_L,
    NQ_KEY_M, NQ_KEY_N, NQ_KEY_O, NQ_KEY_P, NQ_KEY_Q, NQ_KEY_R,
    NQ_KEY_S, NQ_KEY_T, NQ_KEY_U, NQ_KEY_V, NQ_KEY_W, NQ_KEY_X,
    NQ_KEY_Y, NQ_KEY_Z,
    NQ_KEY_0, NQ_KEY_1, NQ_KEY_2, NQ_KEY_3, NQ_KEY_4,
    NQ_KEY_5, NQ_KEY_6, NQ_KEY_7, NQ_KEY_8, NQ_KEY_9,
    NQ_KEY_SPACE, NQ_KEY_ENTER, NQ_KEY_ESCAPE, NQ_KEY_TAB, NQ_KEY_BACKSPACE,
    NQ_KEY_LEFT, NQ_KEY_RIGHT, NQ_KEY_UP, NQ_KEY_DOWN,
    NQ_KEY_LSHIFT, NQ_KEY_RSHIFT, NQ_KEY_LCTRL, NQ_KEY_RCTRL,
    NQ_KEY_LALT, NQ_KEY_RALT,
    NQ_KEY_UNKNOWN = NQ_KEY_COUNT - 1
} NqKey;

typedef enum {
    NQ_EVENT_NONE     = 0,
    NQ_EVENT_KEY_DOWN = 1,
    NQ_EVENT_KEY_UP   = 2,
    NQ_EVENT_QUIT     = 3,
} NqEventKind;

typedef struct {
    NqEventKind kind;
    NqKey       key;
} NqEvent;

typedef struct NqInput NqInput;

NqInput *nq_input_create(void);
void     nq_input_destroy(NqInput *in);

/* Feed one event. Edge transitions are tracked internally; cleared by
 * nq_input_poll_begin() at the start of the next frame. */
void nq_input_process(NqInput *in, NqEvent ev);

/* Per-frame: clears the edge flags from the previous frame. Call BEFORE
 * pumping events for the new frame. */
void nq_input_poll_begin(NqInput *in);

/* Edge queries — true for exactly one frame on the transition. */
bool nq_input_pressed (const NqInput *in, NqKey key);
bool nq_input_released(const NqInput *in, NqKey key);

/* Level query — true while the key is held (after DOWN, until UP). */
bool nq_input_down(const NqInput *in, NqKey key);

/* Quit flag — set when a NQ_EVENT_QUIT arrived during process(). Cleared
 * by poll_begin() with the rest of the edges. */
bool nq_input_quit(const NqInput *in);

/* SDL3 bridge — these are the only SDL-aware pieces. They take an
 * SDL_Keycode (the runtime type is int because we don't want to
 * include SDL3 in this header's consumers). */
NqEvent nq_input_make_keydown_event(int sdl_keycode);
NqEvent nq_input_make_keyup_event  (int sdl_keycode);

#endif /* NQ_INPUT_H */
