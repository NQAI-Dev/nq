#include "nq/input_text.h"
#include "nq/log.h"

#include <SDL3/SDL.h>

#include <string.h>

void nq_input_text_init(NqInputText *t) {
    if (!t) return;
    t->buf[0]   = '\0';
    t->length   = 0;
    t->cursor   = 0;
}

void nq_input_text_set(NqInputText *t, const char *s) {
    if (!t) return;
    if (!s) {
        nq_input_text_init(t);
        return;
    }
    size_t n = strlen(s);
    if (n > NQ_INPUT_TEXT_MAX - 1) n = NQ_INPUT_TEXT_MAX - 1;
    memcpy(t->buf, s, n);
    t->buf[n] = '\0';
    t->length = n;
    t->cursor = n;
}

/* Find the start of the UTF-8 codepoint that ends at or before `pos`.
 * Returns `pos` unchanged if it's already on a codepoint boundary. */
static size_t utf8_prev_boundary(const char *s, size_t pos) {
    if (pos == 0) return 0;
    /* Walk back at most 4 bytes (max UTF-8 length) until we find a
     * leading byte (one whose top 2 bits are not "10" — those are
     * continuation bytes). */
    size_t p = pos;
    for (int i = 0; i < 4 && p > 0; i++) {
        p--;
        unsigned char b = (unsigned char)s[p];
        if (b < 0x80 || b >= 0xC0) {
            return p;
        }
    }
    return 0;
}

int nq_input_text_pump_sdl3_event(NqInputText *t, void *event) {
    if (!t || !event) return 0;
    SDL_Event *e = (SDL_Event *)event;
    switch (e->type) {
    case SDL_EVENT_TEXT_INPUT: {
        /* Append the new UTF-8 text at the cursor.
         * (Replacing instead would require per-frame selection
         * tracking — beyond this tick's scope.) */
        const char *txt = e->text.text;
        size_t add = strlen(txt);
        if (add == 0) return 1;
        /* Make room. */
        if (t->cursor != t->length) {
            /* Insert in the middle: shift right. */
            size_t move = t->length - t->cursor;
            if (t->length + add >= NQ_INPUT_TEXT_MAX) {
                /* Truncate the add to fit. */
                add = NQ_INPUT_TEXT_MAX - 1 - t->length;
            }
            if (add == 0) return 1;
            memmove(t->buf + t->cursor + add,
                    t->buf + t->cursor,
                    move);
        } else if (t->length + add >= NQ_INPUT_TEXT_MAX) {
            add = NQ_INPUT_TEXT_MAX - 1 - t->length;
            if (add == 0) return 1;
        }
        memcpy(t->buf + t->cursor, txt, add);
        t->length += add;
        t->buf[t->length] = '\0';
        t->cursor += add;
        return 1;
    }
    case SDL_EVENT_KEY_DOWN: {
        SDL_Keycode key = e->key.key;
        if (key == SDLK_BACKSPACE && t->cursor > 0) {
            /* Delete the codepoint to the left of the cursor. */
            size_t prev = utf8_prev_boundary(t->buf, t->cursor);
            size_t n = t->cursor - prev;
            memmove(t->buf + prev, t->buf + t->cursor, t->length - t->cursor);
            t->length -= n;
            t->buf[t->length] = '\0';
            t->cursor = prev;
            return 1;
        }
        if (key == SDLK_DELETE && t->cursor < t->length) {
            /* Delete the codepoint to the right. */
            /* Find end of the codepoint starting at cursor. */
            size_t end = t->cursor;
            while (end < t->length) {
                unsigned char b = (unsigned char)t->buf[end];
                int cont_bytes =
                    (b < 0x80) ? 0 :
                    (b < 0xC0) ? 0 :
                    (b < 0xE0) ? 1 :
                    (b < 0xF0) ? 2 : 3;
                end += 1 + cont_bytes;
                if (end > t->length) {
                    end = t->length;  /* malformed sequence — be lenient */
                    break;
                }
                if (cont_bytes == 0 || (b & 0xC0) == 0xC0) break;
            }
            if (end == t->cursor) end = t->cursor + 1;  /* safety */
            size_t n = end - t->cursor;
            memmove(t->buf + t->cursor, t->buf + end, t->length - end);
            t->length -= n;
            t->buf[t->length] = '\0';
            return 1;
        }
        if (key == SDLK_LEFT && t->cursor > 0) {
            t->cursor = utf8_prev_boundary(t->buf, t->cursor);
            return 1;
        }
        if (key == SDLK_RIGHT && t->cursor < t->length) {
            /* Advance one codepoint. */
            size_t p = t->cursor;
            unsigned char b = (unsigned char)t->buf[p];
            int cont_bytes =
                (b < 0x80) ? 0 :
                (b < 0xE0) ? 1 :
                (b < 0xF0) ? 2 : 3;
            p += 1 + cont_bytes;
            if (p > t->length) p = t->length;
            t->cursor = p;
            return 1;
        }
        if (key == SDLK_HOME) { t->cursor = 0; return 1; }
        if (key == SDLK_END)  { t->cursor = t->length; return 1; }
        return 0;
    }
    default:
        return 0;
    }
}
