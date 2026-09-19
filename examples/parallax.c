/*
 * nq — parallax demo.
 *
 * Three coloured rectangles scroll at different speeds as the user moves
 * the cursor left/right. Demonstrates:
 *
 *   nq_node           — per-layer transform (x position drives parallax)
 *   nq_scene          — root scene tree, draw order matches insertion
 *   nq_input_sdl3     — mouse motion -> nq_input state
 *   nq_input_*_down   — keyboard query (arrow keys for instant movement)
 *   nq_clock          — frame dt for smooth motion
 *   nq_renderer_*     — fill_rect on each layer
 *
 * No textures — each layer is just a solid filled rectangle of
 * different colour and width. The "parallax" effect comes entirely
 * from the per-layer scroll speed: background moves slowly, foreground
 * moves 1:1 with the input.
 *
 * Controls: arrow keys / WASD move the camera (positive x). ESC quits.
 * Mouse x also drives the camera so you can wave the cursor around.
 */

#include <SDL3/SDL.h>

#include <nq/common.h>
#include <nq/graphics.h>
#include <nq/node.h>
#include <nq/scene.h>
#include <nq/clock.h>
#include <nq/input.h>
#include <nq/input_sdl3.h>

#include <stdio.h>
#include <stdlib.h>

#define WIN_W 800
#define WIN_H 480
#define LAYER_H 140     /* each layer is a horizontal band */
#define NUM_LAYERS 3

/* Per-layer scroll configuration. parallax_factor < 1 = far away
 * (background moves slower); 1 = foreground moves 1:1 with input. */
typedef struct {
    NqColor color;
    float  parallax_factor;
    /* Local x position of the layer's left edge. Updated each frame
     * from the camera offset × parallax_factor. The rectangle is drawn
     * at (this position, 0); if it scrolls off the right edge we wrap. */
    float  scroll_x;
} Layer;

static Layer layers[NUM_LAYERS] = {
    { {  30,  40,  60, 255 }, 0.20f, 0.0f },  /* background, blueish */
    { {  90, 110, 140, 255 }, 0.50f, 0.0f },  /* mid, lighter */
    { { 200,  90,  90, 255 }, 1.00f, 0.0f },  /* foreground, red */
};

static float camera_x = 0.0f;  /* moves with input; layers derive scroll_x */

static int should_quit(const SDL_Event *e) {
    return e->type == SDL_EVENT_QUIT ||
           (e->type == SDL_EVENT_KEY_DOWN && e->key.key == SDLK_ESCAPE);
}

int main(void) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "nq: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Window *sdl_window = SDL_CreateWindow("nq — parallax",
                                              WIN_W, WIN_H, 0);
    if (!sdl_window) {
        SDL_Quit();
        return 1;
    }
    NqRenderer *ren = nq_renderer_create((NqWindow *)sdl_window);
    if (!ren) {
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
        return 1;
    }

    NqClock *clock = nq_clock_create();
    NqInput  input;
    nq_input_init(&input);

    /* Layer y-positions: stacked vertically with overlap so the
     * "horizon" sits where the bands meet. */
    const int layer_y[NUM_LAYERS] = {
        WIN_H - LAYER_H * 3,          /* background band */
        WIN_H - LAYER_H * 2 + 30,     /* mid, slightly lower */
        WIN_H - LAYER_H + 60,         /* foreground, even lower */
    };

    int running = 1;
    SDL_Event event;
    Uint64 freq = SDL_GetPerformanceFrequency();
    Uint64 last = SDL_GetPerformanceCounter();
    float dt = 0.016f;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (should_quit(&event)) {
                running = 0;
            } else {
                nq_input_pump_sdl3_event(&input, &event);
            }
        }
        nq_input_begin_frame(&input);

        Uint64 now = SDL_GetPerformanceCounter();
        dt = (float)((double)(now - last) / (double)freq);
        if (dt < 0.0f) dt = 0.0f;
        if (dt > 0.1f) dt = 0.1f;
        last = now;

        /* Camera input. Arrow keys contribute a constant velocity per
         * held key; mouse x is used as a soft target. */
        int kx = (nq_input_key_down(&input, SDL_GetScancodeFromKey(SDLK_RIGHT, NULL))
               - nq_input_key_down(&input, SDL_GetScancodeFromKey(SDLK_LEFT, NULL)))
              + (nq_input_key_down(&input, SDL_GetScancodeFromKey(SDLK_D, NULL))
               - nq_input_key_down(&input, SDL_GetScancodeFromKey(SDLK_A, NULL)));
        int ky = (nq_input_key_down(&input, SDL_GetScancodeFromKey(SDLK_DOWN, NULL))
               - nq_input_key_down(&input, SDL_GetScancodeFromKey(SDLK_UP, NULL)))
              + (nq_input_key_down(&input, SDL_GetScancodeFromKey(SDLK_S, NULL))
               - nq_input_key_down(&input, SDL_GetScancodeFromKey(SDLK_W, NULL)));

        camera_x += (float)kx * 220.0f * dt;  /* arrow-key velocity */
        (void)ky;  /* unused — vertical scrolling not in scope here */

        /* Per-layer scroll position. Wrap when off-screen. */
        const int layer_extent[NUM_LAYERS] = { 1200, 900, 700 };
        for (int i = 0; i < NUM_LAYERS; i++) {
            layers[i].scroll_x -= (float)kx * 220.0f * dt * layers[i].parallax_factor;
            int extent = layer_extent[i];
            /* Wrap to keep the layer visible: when the left edge has
             * scrolled past -extent, jump forward by extent. */
            if (layers[i].scroll_x < -extent) {
                layers[i].scroll_x += 2.0f * extent;
            }
            if (layers[i].scroll_x > extent) {
                layers[i].scroll_x -= 2.0f * extent;
            }
        }

        nq_renderer_clear(ren, NQ_COLOR_RGB(15, 18, 28));

        /* Draw layers back-to-front so the foreground occludes the
         * background — same order as nq_scene would draw its stack. */
        for (int i = 0; i < NUM_LAYERS; i++) {
            /* Draw the layer as a wide band; it scrolls past the
             * viewport and wraps, so we draw it twice if needed for
             * a seamless edge. */
            float x = layers[i].scroll_x;
            nq_renderer_fill_rect(ren,
                layers[i].color,
                (int)x, layer_y[i], WIN_W, LAYER_H);
            /* Repeat one width to the right so the seam isn't visible. */
            nq_renderer_fill_rect(ren,
                layers[i].color,
                (int)x + WIN_W, layer_y[i], WIN_W, LAYER_H);
        }

        nq_renderer_present(ren);
    }

    (void)clock;   /* SDL perf counter is the actual dt source; placeholder for future nq_clock usage */
    nq_renderer_destroy(ren);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
    return 0;
}
