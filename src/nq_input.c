#include "nq/input.h"

#include <string.h>

void nq_input_init(NqInput *in) {
    if (!in) return;
    memset(&in->kb, 0, sizeof(in->kb));
    memset(&in->mouse, 0, sizeof(in->mouse));
}

void nq_input_begin_frame(NqInput *in) {
    if (!in) return;
    /* Snapshot of "current" becomes "prev" for edge detection. The actual
     * current state is overwritten by the event pump during the frame. */
    memcpy(&in->kb.prev, &in->kb.current, sizeof(in->kb.prev));
    in->mouse.prev_buttons = in->mouse.buttons;
    in->mouse.rel_x = 0;
    in->mouse.rel_y = 0;
}

void nq_input_set_key(NqInput *in, int scancode, int down) {
    if (!in || scancode < 0 || scancode >= NQ_KEY_MAX) return;
    in->kb.current[scancode] = down ? 1 : 0;
}

void nq_input_set_mouse_pos(NqInput *in, int x, int y) {
    if (!in) return;
    in->mouse.rel_x += (x - in->mouse.x);
    in->mouse.rel_y += (y - in->mouse.y);
    in->mouse.x = x;
    in->mouse.y = y;
}

void nq_input_set_mouse_button(NqInput *in, int button_mask) {
    if (!in) return;
    in->mouse.buttons = button_mask & (NQ_MOUSE_BUTTON_LEFT |
                                      NQ_MOUSE_BUTTON_RIGHT |
                                      NQ_MOUSE_BUTTON_MIDDLE);
}

int nq_input_key_down(const NqInput *in, int scancode) {
    if (!in || scancode < 0 || scancode >= NQ_KEY_MAX) return 0;
    return in->kb.current[scancode];
}

int nq_input_key_pressed(const NqInput *in, int scancode) {
    if (!in || scancode < 0 || scancode >= NQ_KEY_MAX) return 0;
    return in->kb.current[scancode] && !in->kb.prev[scancode];
}

int nq_input_key_released(const NqInput *in, int scancode) {
    if (!in || scancode < 0 || scancode >= NQ_KEY_MAX) return 0;
    return !in->kb.current[scancode] && in->kb.prev[scancode];
}

int nq_input_mouse_x(const NqInput *in)  { return in ? in->mouse.x : 0; }
int nq_input_mouse_y(const NqInput *in)  { return in ? in->mouse.y : 0; }
int nq_input_mouse_dx(const NqInput *in) { return in ? in->mouse.rel_x : 0; }
int nq_input_mouse_dy(const NqInput *in) { return in ? in->mouse.rel_y : 0; }

int nq_input_mouse_down(const NqInput *in, int button_mask) {
    if (!in) return 0;
    return (in->mouse.buttons & button_mask) == button_mask;
}

int nq_input_mouse_pressed(const NqInput *in, int button_mask) {
    if (!in) return 0;
    int now  = in->mouse.buttons & button_mask;
    int prev = in->mouse.prev_buttons & button_mask;
    return now && !prev;
}
