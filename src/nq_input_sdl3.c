#include "nq/input_sdl3.h"
#include "nq/log.h"

int nq_input_pump_sdl3_event(NqInput *in, const SDL_Event *event) {
    if (!in || !event) return 0;
    switch (event->type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
        /* SDL_GetScancodeFromKey returns SDL_Scancode which is the same
         * numeric domain as our internal key table. Convert with the
         * explicit cast so future SDL3 changes can't silently break us. */
        SDL_Keycode key = event->key.key;
        SDL_Scancode sc = SDL_GetScancodeFromKey(key);
        int scancode = (sc >= 0) ? (int)sc : -1;
        nq_input_set_key(in, scancode, event->type == SDL_EVENT_KEY_DOWN);
        return 1;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP: {
        int mask = 0;
        if (event->button.button == SDL_BUTTON_LEFT)   mask |= NQ_MOUSE_BUTTON_LEFT;
        if (event->button.button == SDL_BUTTON_RIGHT)  mask |= NQ_MOUSE_BUTTON_RIGHT;
        if (event->button.button == SDL_BUTTON_MIDDLE) mask |= NQ_MOUSE_BUTTON_MIDDLE;
        if (mask == 0) return 0;
        /* Read current button state from the event's button field
         * (SDL3's event.button.down is reliable on the *Down variant,
         * but on *Up we can't trust it — track state by toggling). */
        static int current_buttons = 0;
        if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) current_buttons |= mask;
        else                                            current_buttons &= ~mask;
        nq_input_set_mouse_button(in, current_buttons);
        return 1;
    }
    case SDL_EVENT_MOUSE_MOTION: {
        nq_input_set_mouse_pos(in, (int)event->motion.x, (int)event->motion.y);
        return 1;
    }
    default:
        return 0;
    }
}
