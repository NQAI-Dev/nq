/*
 * nq — input state.
 *
 * Tracks keyboard + mouse state across frames. Edge detection (pressed /
 * released) is computed against the previous frame's snapshot, so callers
 * must call nq_input_begin_frame() at the start of each render tick before
 * SDL3 (or whichever backend) pumps events that update the current state.
 *
 * Mouse deltas accumulate between begin_frame() calls and are read with
 * nq_input_mouse_dx() / nq_input_mouse_dy(). This matches SDL3's relative
 * motion model so a hover during one frame registers as dx/dy in the next.
 *
 * Intentionally engine-internal — no SDL3 types here. The backend (SDL3
 * events, raw input, network events from a remote controller, …) pumps
 * values into NqInput via nq_input_pump_* setters. Keeping this in pure C
 * keeps the module unit-testable without a display server.
 */
#ifndef NQ_INPUT_H
#define NQ_INPUT_H

#include <stdint.h>

#define NQ_KEY_MAX       512
#define NQ_MOUSE_BUTTON_LEFT   0x1
#define NQ_MOUSE_BUTTON_RIGHT  0x2
#define NQ_MOUSE_BUTTON_MIDDLE 0x4

typedef struct {
    uint8_t current[NQ_KEY_MAX];
    uint8_t prev[NQ_KEY_MAX];
} NqKeyboard;

typedef struct {
    int  x;
    int  y;
    int  rel_x;     /* accumulated since last begin_frame() */
    int  rel_y;
    int  buttons;   /* bitmask of NQ_MOUSE_BUTTON_* */
    int  prev_buttons;
} NqMouse;

typedef struct {
    NqKeyboard kb;
    NqMouse    mouse;
} NqInput;

void nq_input_init(NqInput *in);

/* Call at the top of each frame, before the event pump. Resets relative
 * mouse deltas to 0. Keyboard prev snapshot is updated here so edge
 * detection against the just-finished frame is correct. */
void nq_input_begin_frame(NqInput *in);

/* Backend setters — called by the platform event pump, once per
 * physical event. Key scancodes follow SDL3 convention (1..512). */
void nq_input_set_key(NqInput *in, int scancode, int down);
void nq_input_set_mouse_pos(NqInput *in, int x, int y);
void nq_input_set_mouse_button(NqInput *in, int button_mask);

/* Queries. `pressed` = edge down this frame; `released` = edge up.
 * `down` = currently held (regardless of when pressed). */
int nq_input_key_down(const NqInput *in, int scancode);
int nq_input_key_pressed(const NqInput *in, int scancode);
int nq_input_key_released(const NqInput *in, int scancode);

int nq_input_mouse_x(const NqInput *in);
int nq_input_mouse_y(const NqInput *in);
int nq_input_mouse_dx(const NqInput *in);
int nq_input_mouse_dy(const NqInput *in);
int nq_input_mouse_down(const NqInput *in, int button_mask);
int nq_input_mouse_pressed(const NqInput *in, int button_mask);

#endif /* NQ_INPUT_H */
