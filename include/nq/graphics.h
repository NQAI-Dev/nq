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
