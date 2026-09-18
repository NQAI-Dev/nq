/*
 * nq — graphics primitives.
 *
 * Public renderer API. SDL3 is the only supported backend at the moment;
 * the NqRenderer struct holds an SDL_Renderer* and is intentionally opaque so
 * non-SDL backends can be swapped in without touching consumers.
 *
 * All color components are 0..255; convenience macros NQ_COLOR_RGB / NQ_COLOR_RGBA
 * build the 4-channel struct literal at the call site. Functions return 0 on
 * success and -1 if `r` is NULL or the underlying SDL call failed.
 */
#ifndef NQ_GRAPHICS_H
#define NQ_GRAPHICS_H

#include <stdint.h>

typedef struct NqRenderer NqRenderer;
typedef struct NqWindow   NqWindow;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} NqColor;

#define NQ_COLOR_RGB(r, g, b)     ((NqColor){(uint8_t)(r), (uint8_t)(g), (uint8_t)(b), 255})
#define NQ_COLOR_RGBA(r, g, b, a) ((NqColor){(uint8_t)(r), (uint8_t)(g), (uint8_t)(b), (uint8_t)(a)})

NqRenderer *nq_renderer_create(NqWindow *window);
void       nq_renderer_destroy(NqRenderer *r);
int        nq_renderer_clear(NqRenderer *r, NqColor color);
int        nq_renderer_set_draw_color(NqRenderer *r, NqColor color);
int        nq_renderer_fill_rect(NqRenderer *r, NqColor color,
                                 int x, int y, int w, int h);

/* Filled circle (disk) centred at (cx, cy) with radius r.
 * Backing implementation: SDL_RenderFillCircle (SDL3 >= 3.0).
 * Returns 0 on success, -1 on invalid args or SDL failure. */
int        nq_renderer_fill_circle(NqRenderer *renderer, NqColor color,
                                   int cx, int cy, int radius);

/* Circle outline (1px ring) centred at (cx, cy) with radius r.
 * Backing implementation: SDL_RenderCircle (SDL3 >= 3.0).
 * Returns 0 on success, -1 on invalid args or SDL failure. */
int        nq_renderer_draw_circle(NqRenderer *renderer, NqColor color,
                                   int cx, int cy, int radius);

/* Line from (x1, y1) to (x2, y2) in screen coords.
 * Backing implementation: SDL_RenderLine (SDL3 >= 3.0).
 * Returns 0 on success, -1 on invalid args or SDL failure.
 * Note: a 1px line at integer coords is rendered half-on / half-off
 * the pixel — for crisp lines use x+1 / y+1 or NqRenderer::clip_rect. */
int        nq_renderer_draw_line(NqRenderer *renderer, NqColor color,
                                 int x1, int y1, int x2, int y2);

/* Triangle outline: three edges (x0,y0)→(x1,y1)→(x2,y2)→(x0,y0).
 * Vertices may be in any winding order.
 * Returns 0 on success, -1 if renderer is NULL or any edge draw fails. */
int        nq_renderer_draw_triangle(NqRenderer *renderer, NqColor color,
                                     int x0, int y0,
                                     int x1, int y1,
                                     int x2, int y2);

/* Filled triangle using scanline rasterization (CPU-side, no GPU textures).
 * Vertices may be in any winding order; concave/degenerate inputs are safe
 * (degenerate collapses to a line or point and returns 0).
 * Returns 0 on success, -1 if renderer is NULL or a scanline draw fails. */
int        nq_renderer_fill_triangle(NqRenderer *renderer, NqColor color,
                                     int x0, int y0,
                                     int x1, int y1,
                                     int x2, int y2);

void       nq_renderer_present(NqRenderer *r);

/* Backend escape hatch: returns the underlying SDL_Renderer* for code
 * that needs direct SDL access. nq_texture.c uses this to drive
 * SDL_RenderTexture. If we ever swap to a different backend, this is
 * the only NqRenderer accessor that needs to change.
 *
 * Forward declaration only — full SDL3 types stay out of this header so
 * other consumers can include nq/graphics.h without pulling SDL3 in. */
typedef struct SDL_Renderer SDL_Renderer;
SDL_Renderer *nq_renderer_sdl(NqRenderer *r);


#endif /* NQ_GRAPHICS_H */
