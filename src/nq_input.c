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
    in->mouse.wheel = 0;
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

void nq_input_set_mouse_wheel(NqInput *in, int delta) {
    if (!in) return;
    /* Wheel deltas accumulate across event-pump calls within a single
     * frame so multiple scroll events (e.g. fast trackpad swipe) don't
     * silently overwrite each other. */
    in->mouse.wheel += delta;
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

int nq_input_mouse_wheel(const NqInput *in) {
    return in ? in->mouse.wheel : 0;
}

int nq_input_mouse_inside_rect(const NqInput *in, int x, int y, int w, int h) {
    if (!in) return 0;
    /* Hit-test against the half-open rect convention used by nq_rect_contains:
     * x in [x, x+w), y in [y, y+h). The rect is empty (w<=0 || h<=0)
     * returns false so empty UI elements never register as hovered. */
    if (w <= 0 || h <= 0) return 0;
    return in->mouse.x >= x && in->mouse.x < x + w
        && in->mouse.y >= y && in->mouse.y < y + h;
}

void nq_input_clear(NqInput *in) {
    if (!in) return;
    /* Zero both keyboard snapshots so no "still pressed" edge fires on
     * the next begin_frame(); zero mouse position and deltas so a stale
     * cursor doesn't bleed into the new scene. */
    memset(&in->kb, 0, sizeof(in->kb));
    memset(&in->mouse, 0, sizeof(in->mouse));
}
