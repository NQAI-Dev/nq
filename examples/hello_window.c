/*
 * nq — first runnable example.
 *
 * Opens a 640x480 SDL3 window, draws a single bouncing colored square that
 * bounces off the window edges using simple AABB reflection. Demonstrates
 * the nq_renderer_* public API rather than calling SDL3 directly — the
 * engine owns the backend; the example is just the consumer.
 *
 * Controls: ESC or window-close to quit.
 */

#include <SDL3/SDL.h>
#include <nq/graphics.h>

#include <stdio.h>

#define NQ_WINDOW_W   640
#define NQ_WINDOW_H   480
#define NQ_WINDOW_TTL "nq — bouncing square"
#define NQ_BOX_W      80
#define NQ_BOX_H      80
/* Pixels per second; the example's choice, not the engine's. */
#define NQ_BOX_VX     140
#define NQ_BOX_VY      90

static int nq_should_quit(const SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) {
        return 1;
    }
    if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_ESCAPE) {
        return 1;
    }
    return 0;
}

/* Clamp `pos` to [0, max] keeping the box inside the window on that axis. */
static int nq_clamp(int pos, int box_size, int max) {
    if (pos < 0) {
        return 0;
    }
    if (pos + box_size > max) {
        return max - box_size;
    }
    return pos;
}

int main(void) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "nq: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *sdl_window = SDL_CreateWindow(NQ_WINDOW_TTL,
                                              NQ_WINDOW_W, NQ_WINDOW_H, 0);
    if (!sdl_window) {
        fprintf(stderr, "nq: SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    /* NqWindow is opaque; we pass the SDL_Window* through as void* for now. */
    NqRenderer *renderer = nq_renderer_create((NqWindow *)sdl_window);
    if (!renderer) {
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
        return 1;
    }

    int running = 1;
    SDL_Event event;
    Uint64 last_ticks = SDL_GetTicks();
    int box_x = 40;
    int box_y = 40;
    int vel_x = NQ_BOX_VX;
    int vel_y = NQ_BOX_VY;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (nq_should_quit(&event)) {
                running = 0;
            }
        }

        Uint64 now_ticks = SDL_GetTicks();
        float dt = (float)(now_ticks - last_ticks) / 1000.0f;
        last_ticks = now_ticks;

        box_x += (int)(vel_x * dt);
        box_y += (int)(vel_y * dt);

        if (box_x <= 0 || box_x + NQ_BOX_W >= NQ_WINDOW_W) {
            vel_x = -vel_x;
            box_x = nq_clamp(box_x, NQ_BOX_W, NQ_WINDOW_W);
        }
        if (box_y <= 0 || box_y + NQ_BOX_H >= NQ_WINDOW_H) {
            vel_y = -vel_y;
            box_y = nq_clamp(box_y, NQ_BOX_H, NQ_WINDOW_H);
        }

        /* Engine API calls from here on — no SDL3 in the example body. */
        nq_renderer_clear(renderer, NQ_COLOR_RGB(20, 24, 32));
        nq_renderer_fill_rect(renderer,
            NQ_COLOR_RGB(
                (uint8_t)((box_x * 255) / NQ_WINDOW_W),
                (uint8_t)((box_y * 255) / NQ_WINDOW_H),
                (uint8_t)200),
            box_x, box_y, NQ_BOX_W, NQ_BOX_H);
        nq_renderer_present(renderer);
    }

    nq_renderer_destroy(renderer);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
    return 0;
}
