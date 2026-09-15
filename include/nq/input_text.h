/*
 * nq — text input buffer.
 *
 * Holds a UTF-8 text buffer + cursor position for a single text-input
 * field (player name, chat, save-game label, etc). Pumped via
 * nq_input_text_pump_sdl3_event() from an SDL3 event loop; the public
 * API is backend-agnostic.
 *
 * Capacity: fixed upper bound (NQ_INPUT_TEXT_MAX = 256 bytes including
 * the NUL terminator). For a real game's "name" field this is plenty;
 * if a project needs longer input, swap to a heap buffer without
 * changing the public API.
 *
 * Cursor semantics: `cursor` is the byte index of the insertion point
 * (0..length). 0 == insert at the start; length == append. Renderers
 * that want a "caret at column N" usually want cursor-1 in their own
 * cursor model (since cursor points to the gap, not to a character).
 */
#ifndef NQ_INPUT_TEXT_H
#define NQ_INPUT_TEXT_H

#include <stddef.h>

/* Forward declaration so this header stays SDL3-free;
 * the .c file does the SDL_StartTextInput dance. */
typedef union SDL_Event SDL_Event;

#define NQ_INPUT_TEXT_MAX 256

typedef struct {
    char    buf[NQ_INPUT_TEXT_MAX];
    size_t  length;     /* bytes excluding NUL terminator */
    size_t  cursor;     /* insertion offset, 0..length */
} NqInputText;

void nq_input_text_init(NqInputText *t);

/* Replace the current buffer with a new UTF-8 string. Truncates if
 * longer than NQ_INPUT_TEXT_MAX-1. Cursor lands at end. */
void nq_input_text_set(NqInputText *t, const char *s);

/* Insert / replace at the cursor. Handles ASCII editing keys
 * (Backspace, Delete, Home, End, Left, Right) — multi-byte UTF-8 is
 * passed through byte-by-byte via pump_sdl3_event. Returns 1 if the
 * event was consumed, 0 if it wasn't an editing key (caller decides
 * what to do — usually means it's not for the text field). */
int  nq_input_text_pump_sdl3_event(NqInputText *t, void *event);

#endif /* NQ_INPUT_TEXT_H */
