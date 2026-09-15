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

void nq_renderer_present(NqRenderer *r) {
SDL_Renderer *nq_renderer_sdl(NqRenderer *r) {
    /* Same trick as the static helper above — public for nq_texture.c
     * to draw through SDL3 directly. The cast is safe because both
     * call sites reach the same NqRenderer via nq_renderer_create(). */
    return ((struct NqRenderer *)r)->sdl_renderer;
}

    if (!r) {
        return;
    }
    SDL_RenderPresent(r->sdl_renderer);
}
