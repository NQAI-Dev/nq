#include "nq/input.h"

#include <stdlib.h>
#include <string.h>

struct NqInput {
    /* Bitset: bit i set ↔ key i is currently held (DOWN without UP). */
    uint8_t down[NQ_KEY_COUNT / 8];
    /* Bitset: transitions in the current frame. */
    uint8_t pressed [NQ_KEY_COUNT / 8];
    uint8_t released[NQ_KEY_COUNT / 8];
    bool quit;
};

static void set_bit(uint8_t *bits, int idx)   { bits[idx / 8] |=  (uint8_t)(1u << (idx % 8)); }
static void clr_bit(uint8_t *bits, int idx)   { bits[idx / 8] &= ~(uint8_t)(1u << (idx % 8)); }
static bool get_bit(const uint8_t *bits, int idx) { return (bits[idx / 8] >> (idx % 8)) & 1u; }

static bool in_range(NqKey key) { return (int)key >= 0 && (int)key < NQ_KEY_COUNT; }

NqInput *nq_input_create(void) {
    NqInput *in = calloc(1, sizeof(NqInput));
    return in;
}

void nq_input_destroy(NqInput *in) {
    free(in);
}

void nq_input_poll_begin(NqInput *in) {
    if (!in) return;
    memset(in->pressed,  0, sizeof(in->pressed));
    memset(in->released, 0, sizeof(in->released));
    in->quit = false;
}

void nq_input_process(NqInput *in, NqEvent ev) {
    if (!in) return;
    switch (ev.kind) {
        case NQ_EVENT_KEY_DOWN:
            if (ev.key == NQ_KEY_NONE) return;
            set_bit(in->down, ev.key);
            set_bit(in->pressed, ev.key);
            break;
        case NQ_EVENT_KEY_UP:
            if (ev.key == NQ_KEY_NONE) return;
            clr_bit(in->down, ev.key);
            set_bit(in->released, ev.key);
            break;
        case NQ_EVENT_QUIT:
            in->quit = true;
            break;
        case NQ_EVENT_NONE:
        default:
            break;
    }
}

bool nq_input_pressed(const NqInput *in, NqKey key) {
    if (!in || !in_range(key)) return false;
    return get_bit(in->pressed, key);
}

bool nq_input_released(const NqInput *in, NqKey key) {
    if (!in || !in_range(key)) return false;
    return get_bit(in->released, key);
}

bool nq_input_down(const NqInput *in, NqKey key) {
    if (!in || !in_range(key)) return false;
    return get_bit(in->down, key);
}

bool nq_input_quit(const NqInput *in) {
    if (!in) return false;
    return in->quit;
}

/* SDL3 → NqKey. Returns NQ_KEY_NONE for unmapped scancodes. The
 * caller (typically the example) is responsible for the SDL events
 * loop; we don't reach into SDL ourselves.
 */
static NqKey nq_sdl_keycode_to_nq_key(int sdl_keycode) {
    if (sdl_keycode >= 'a' && sdl_keycode <= 'z') {
        return (NqKey)(NQ_KEY_A + (sdl_keycode - 'a'));
    }
    if (sdl_keycode >= '0' && sdl_keycode <= '9') {
        return (NqKey)(NQ_KEY_0 + (sdl_keycode - '0'));
    }
    /* SDL3 special keys */
    switch (sdl_keycode) {
        case 0x20: return NQ_KEY_SPACE;
        case 0x0D: return NQ_KEY_ENTER;
        case 0x09: return NQ_KEY_TAB;
        case 0x08: return NQ_KEY_BACKSPACE;
        case 0x1B: return NQ_KEY_ESCAPE;
        case 1073741906: return NQ_KEY_LEFT;
        case 1073741903: return NQ_KEY_RIGHT;
        case 1073741904: return NQ_KEY_UP;
        case 1073741905: return NQ_KEY_DOWN;
        case 1073742049: return NQ_KEY_LSHIFT;
        case 1073742050: return NQ_KEY_RSHIFT;
        case 1073742048: return NQ_KEY_LCTRL;
        case 1073742052: return NQ_KEY_RCTRL;
        case 1073742054: return NQ_KEY_LALT;
        case 1073742057: return NQ_KEY_RALT;
    }
    return NQ_KEY_NONE;
}

NqEvent nq_input_make_keydown_event(int sdl_keycode) {
    NqEvent ev = { NQ_EVENT_NONE, NQ_KEY_NONE };
    ev.kind = NQ_EVENT_KEY_DOWN;
    ev.key  = nq_sdl_keycode_to_nq_key(sdl_keycode);
    return ev;
}

NqEvent nq_input_make_keyup_event(int sdl_keycode) {
    NqEvent ev = { NQ_EVENT_NONE, NQ_KEY_NONE };
    ev.kind = NQ_EVENT_KEY_UP;
    ev.key  = nq_sdl_keycode_to_nq_key(sdl_keycode);
    return ev;
}
