/*
 * nq — text rendering.
 *
 * Built-in 5x7 bitmap font for debug / dev HUDs. NqText holds a glyph
 * atlas (one region per glyph, keyed by character), a per-NqRenderer
 * backing texture, and per-call draw state (color, position).
 *
 * The font data is hardcoded — no external file dependency, so CI can
 * run the example without fixture assets. A real game would replace
 * this with TTF rasterization (e.g. SDL3_ttf) in a follow-up tick.
 *
 * Glyph coverage: ASCII 0x20..0x7E (printable, 95 glyphs). Anything
 * outside renders as '?'.
 */
#ifndef NQ_TEXT_H
#define NQ_TEXT_H

#include <stdint.h>

#include "nq/atlas.h"
#include "nq/texture.h"

typedef struct NqText NqText;

NqText *nq_text_create(NqRenderer *ren);
void    nq_text_destroy(NqText *t);

/* Color for subsequent draws. RGBA bytes. */
void nq_text_set_color(NqText *t, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/* Draw a string at (x, y); top-left baseline at y. Newlines wrap to
 * the next line at the font's line height. Returns the x position after
 * the last drawn character (useful for cursor / caret logic). */
int  nq_text_draw(NqText *t, const char *s, int x, int y);

/* Query: width and height of a glyph for the active font. */
int  nq_text_glyph_w(NqText *t, char c);
int  nq_text_glyph_h(NqText *t, char c);
int  nq_text_line_h(NqText *t);

#endif /* NQ_TEXT_H */
