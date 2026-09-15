/*
 * nq — SDL3 → NqInput bridge.
 *
 * Pumps a single SDL_Event into an NqInput. Split out from
 * include/nq/input.h so consumers that don't link SDL3 don't have to
 * pull it in transitively.
 *
 * SDL3 keyboard scancodes map directly to NqInput's 1..NQ_KEY_MAX slot
 * via SDL_GetScancodeFromKey(). Mouse buttons map 1:1 to NQ_MOUSE_BUTTON_*.
 *
 * Consumers typically loop over SDL_PollEvent and call this on each
 * event they want to forward to the engine; events they don't care about
 * can be ignored.
 */
#ifndef NQ_INPUT_SDL3_H
#define NQ_INPUT_SDL3_H

#include <SDL3/SDL.h>

#include "nq/input.h"

/* Returns 1 if the event was consumed (a known input event), 0 if it was
 * ignored (window event, system event, etc — caller decides what to do). */
int nq_input_pump_sdl3_event(NqInput *in, const SDL_Event *event);

#endif /* NQ_INPUT_SDL3_H */
