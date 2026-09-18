#ifndef NQ_GRAPHICS_EXT_H
#define NQ_GRAPHICS_EXT_H

#include "nq/graphics.h"
#include "nq/rect.h"

int nq_renderer_fill_round_rect(NqRenderer *r, NqColor color, NqRect rect, int radius);
int nq_renderer_draw_round_rect(NqRenderer *r, NqColor color, NqRect rect, int radius);

#endif
