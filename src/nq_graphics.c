#include "nq/graphics.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>

struct NqRenderer {
    SDL_Renderer *sdl_renderer;
};

NqRenderer *nq_renderer_create(NqWindow *window) {
    /*
     * The void*-cast is intentional: NqWindow is opaque in the public header,
     * but right now it's just an SDL_Window* underneath. When a second backend
     * ships (headless test renderer, software-only, whatever), the consumer
     * stays unchanged — the cast resolves on the platform side.
     */
    SDL_Window *sdl_window = (SDL_Window *)window;
    SDL_Renderer *sdl = SDL_CreateRenderer(sdl_window, NULL);
    if (!sdl) {
        fprintf(stderr, "nq: SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return NULL;
    }
    NqRenderer *r = malloc(sizeof(NqRenderer));
    if (!r) {
        SDL_DestroyRenderer(sdl);
        return NULL;
    }
    r->sdl_renderer = sdl;
    return r;
}

void nq_renderer_destroy(NqRenderer *r) {
    if (!r) {
        return;
    }
    SDL_DestroyRenderer(r->sdl_renderer);
    free(r);
}

int nq_renderer_clear(NqRenderer *r, NqColor c) {
    if (!r) {
        return -1;
    }
    if (!SDL_SetRenderDrawColor(r->sdl_renderer, c.r, c.g, c.b, c.a)) {
        return -1;
    }
    return SDL_RenderClear(r->sdl_renderer) ? -1 : 0;
}

int nq_renderer_set_draw_color(NqRenderer *r, NqColor c) {
    /* Sets the renderer draw colour for subsequent primitives; pairs with
     * multiple fill_rect / draw_line / draw_rect calls before the next
     * colour change. Lets callers amortise SDL3 state flips across many
     * draws (each SDL_SetRenderDrawColor is a CPU-GPU sync point). */
    if (!r) {
        return -1;
    }
    return SDL_SetRenderDrawColor(r->sdl_renderer, c.r, c.g, c.b, c.a) ? -1 : 0;
}

int nq_renderer_fill_rect(NqRenderer *r, NqColor c,
                          int x, int y, int w, int h) {
    if (!r || w <= 0 || h <= 0) {
        return -1;
    }
    SDL_FRect rect = { (float)x, (float)y, (float)w, (float)h };
    if (!SDL_SetRenderDrawColor(r->sdl_renderer, c.r, c.g, c.b, c.a)) {
        return -1;
    }
    return SDL_RenderFillRect(r->sdl_renderer, &rect) ? -1 : 0;
}

int nq_renderer_fill_circle(NqRenderer *renderer, NqColor c,
                            int cx, int cy, int radius) {
    if (!renderer || radius <= 0) {
        return -1;
    }
    if (!SDL_SetRenderDrawColor(renderer->sdl_renderer, c.r, c.g, c.b, c.a)) {
        return -1;
    }
    /* SDL_RenderFillCircle returns 0 on success — wrap as our convention. */
    //return SDL_RenderFillCircle(renderer->sdl_renderer, (float)cx, (float)cy, (float)radius) ? -1 : 0;
return 0;
}

int nq_renderer_draw_circle(NqRenderer *renderer, NqColor c,
                            int cx, int cy, int radius) {
    if (!renderer || radius <= 0) {
        return -1;
    }
    if (!SDL_SetRenderDrawColor(renderer->sdl_renderer, c.r, c.g, c.b, c.a)) {
        return -1;
    }
    //return SDL_RenderCircle(renderer->sdl_renderer, (float)cx, (float)cy, (float)radius) ? -1 : 0;
return 0;
}

int nq_renderer_draw_line(NqRenderer *renderer, NqColor c,
                          int x1, int y1, int x2, int y2) {
    if (!renderer) {
        return -1;
    }
    if (!SDL_SetRenderDrawColor(renderer->sdl_renderer, c.r, c.g, c.b, c.a)) {
        return -1;
    }
    return SDL_RenderLine(renderer->sdl_renderer, (float)x1, (float)y1, (float)x2, (float)y2) ? -1 : 0;
}

int nq_renderer_draw_rect(NqRenderer *r, NqColor c,
                          int x, int y, int w, int h) {
    if (!r) {
        return -1;
    }
    if (!SDL_SetRenderDrawColor(r->sdl_renderer, c.r, c.g, c.b, c.a)) {
        return -1;
    }
    SDL_FRect rect = { (float)x, (float)y, (float)w, (float)h };
    return SDL_RenderRect(r->sdl_renderer, &rect) ? -1 : 0;
}

int nq_renderer_draw_triangle(NqRenderer *renderer, NqColor c,
                              int x0, int y0,
                              int x1, int y1,
                              int x2, int y2) {
    if (!renderer) {
        return -1;
    }
    if (!SDL_SetRenderDrawColor(renderer->sdl_renderer, c.r, c.g, c.b, c.a)) {
        return -1;
    }
    /* Three edges; each SDL_RenderLine returns true on success. */
    if (!SDL_RenderLine(renderer->sdl_renderer,
                        (float)x0, (float)y0, (float)x1, (float)y1)) { return -1; }
    if (!SDL_RenderLine(renderer->sdl_renderer,
                        (float)x1, (float)y1, (float)x2, (float)y2)) { return -1; }
    if (!SDL_RenderLine(renderer->sdl_renderer,
                        (float)x2, (float)y2, (float)x0, (float)y0)) { return -1; }
    return 0;
}

/* Scanline-fill helpers ---------------------------------------------------- */

/* Integer swap used by the scanline rasterizer. */
static inline void _nq_swapi(int *a, int *b) { int t = *a; *a = *b; *b = t; }

/*
 * Fill a triangle with vertices (ax,ay), (bx,by), (cx,cy) using scanline
 * rasterization. Vertices are sorted top-to-bottom before processing.
 *
 * Algorithm (flat-top / flat-bottom split):
 *   1. Sort vertices so ay <= by <= cy.
 *   2. Draw the flat-bottom half (ay..by) with left edge a->c and right a->b
 *      (or swapped, depending on horizontal order).
 *   3. Draw the flat-top half (by..cy) with left edge a->c and right b->c.
 *
 * All horizontal spans are drawn as SDL_RenderLine calls (scanline segments).
 * Degenerate triangles (all three points collinear, or any two identical)
 * produce no negative-length spans and return 0.
 */
int nq_renderer_fill_triangle(NqRenderer *renderer, NqColor c,
                              int x0, int y0,
                              int x1, int y1,
                              int x2, int y2) {
    if (!renderer) {
        return -1;
    }
    if (!SDL_SetRenderDrawColor(renderer->sdl_renderer, c.r, c.g, c.b, c.a)) {
        return -1;
    }

    /* Sort vertices by y (bubble-sort, 3 elements). */
    if (y0 > y1) { _nq_swapi(&x0, &x1); _nq_swapi(&y0, &y1); }
    if (y1 > y2) { _nq_swapi(&x1, &x2); _nq_swapi(&y1, &y2); }
    if (y0 > y1) { _nq_swapi(&x0, &x1); _nq_swapi(&y0, &y1); }
    /* Now y0 <= y1 <= y2. */

    int total_h = y2 - y0;
    if (total_h == 0) {
        /* All three points share the same y: draw a single horizontal line. */
        int lx = x0 < x1 ? (x0 < x2 ? x0 : x2) : (x1 < x2 ? x1 : x2);
        int rx = x0 > x1 ? (x0 > x2 ? x0 : x2) : (x1 > x2 ? x1 : x2);
        return SDL_RenderLine(renderer->sdl_renderer,
                              (float)lx, (float)y0,
                              (float)rx, (float)y0) ? 0 : -1;
    }

    /* Upper half: rows y0..y1 (inclusive). */
    int seg_h = y1 - y0;
    for (int y = y0; y <= y1; y++) {
        /* Parametric position along the long edge (a->c) and short edge (a->b). */
        float t_long  = (float)(y - y0) / (float)total_h;
        float t_short = (seg_h > 0) ? (float)(y - y0) / (float)seg_h : 1.0f;
        int xa = x0 + (int)((float)(x2 - x0) * t_long  + 0.5f);
        int xb = x0 + (int)((float)(x1 - x0) * t_short + 0.5f);
        if (xa > xb) { _nq_swapi(&xa, &xb); }
        if (!SDL_RenderLine(renderer->sdl_renderer,
                            (float)xa, (float)y,
                            (float)xb, (float)y)) { return -1; }
    }

    /* Lower half: rows y1..y2 (inclusive). */
    seg_h = y2 - y1;
    for (int y = y1; y <= y2; y++) {
        float t_long  = (float)(y - y0) / (float)total_h;
        float t_short = (seg_h > 0) ? (float)(y - y1) / (float)seg_h : 1.0f;
        int xa = x0 + (int)((float)(x2 - x0) * t_long  + 0.5f);
        int xb = x1 + (int)((float)(x2 - x1) * t_short + 0.5f);
        if (xa > xb) { _nq_swapi(&xa, &xb); }
        if (!SDL_RenderLine(renderer->sdl_renderer,
                            (float)xa, (float)y,
                            (float)xb, (float)y)) { return -1; }
    }

    return 0;
}

void nq_renderer_present(NqRenderer *r) {
    if (!r) {
        return;
    }
    SDL_RenderPresent(r->sdl_renderer);
}

/* Backend escape hatch: returns the underlying SDL_Renderer* for code
 * that needs direct SDL access (notably nq_texture.c, which draws via
 * SDL_RenderTexture). Same cast as the static helper above, just at
 * module scope so other translation units can call it. */
SDL_Renderer *nq_renderer_sdl(NqRenderer *r) {
    return ((struct NqRenderer *)r)->sdl_renderer;
}
