#ifndef NQ_GRAPHICS_EXT_H
#define NQ_GRAPHICS_EXT_H

#include "nq/graphics.h"
#include "nq/rect.h"

int nq_renderer_fill_round_rect(NqRenderer *r, NqColor color, NqRect rect, int radius);
int nq_renderer_draw_round_rect(NqRenderer *r, NqColor color, NqRect rect, int radius);

#endif

/* Optional utility function for a crosshair */
int nq_renderer_draw_cross(NqRenderer *r, NqColor color, int x, int y, int size);
/* Thick line utility using rotated rectangle */
int nq_renderer_draw_thick_line(NqRenderer *r, NqColor color, int x1, int y1, int x2, int y2, int thickness);
/* Draws a discrete point */
int nq_renderer_draw_point(NqRenderer *r, NqColor color, int x, int y);
/* Utility to draw a grid over a rectangle (useful for debugging, tilemaps, etc.) */
int nq_renderer_draw_grid(NqRenderer *r, NqColor color, int x, int y, int w, int h, int cell_w, int cell_h);
