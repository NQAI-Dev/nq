/*
 * nq — controlled_square example.
 *
 * A square the user controls: arrow keys / WASD add velocity, ESC quits.
 * Demonstrates the full input pipeline: SDL_PollEvent → nq_input_pump_sdl3_event
 * → nq_input_* queries (per-frame) → physics update → render.
 *
 * Differences from animated_square: this example actually consumes input,
 * the colour doesn't animate (it stays orange so the controls are easy to
 * read), and there's no action_manager — pure event-driven update.
 */

#include <SDL3/SDL.h>

#include <nq/common.h>
#include <nq/graphics.h>
#include <nq/input.h>
#include <nq/input_sdl3.h>

#include <stdio.h>
#include <stdlib.h>

#define WIN_W 640
#define WIN_H 480
#define SQUARE_SIZE 40
#define ACCEL       900.0f   /* px / s^2 */
#define MAX_SPEED    400.0f  /* px / s */
#define FRICTION       4.0f  /* velocity *= exp(-friction * dt) decay */

static int box_x = (WIN_W - SQUARE_SIZE) / 2;
static int box_y = (WIN_H - SQUARE_SIZE) / 2;
static float vel_x = 0.0f;
static float vel_y = 0.0f;

static int should_quit(const SDL_Event *e) {
    return e->type == SDL_EVENT_QUIT ||
           (e->type == SDL_EVENT_KEY_DOWN && e->key.key == SDLK_ESCAPE);
}

/* Maps a held key to an axis sign (-1, 0, +1). Both arrow keys and
 * WASD are recognised so the example is comfortable on any layout. */
static int axis_sign(const NqInput *in, SDL_Keycode neg, SDL_Keycode pos) {
    int n = nq_input_key_down(in, SDL_GetScancodeFromKey(neg));
    int p = nq_input_key_down(in, SDL_GetScancodeFromKey(pos));
    return (int)p - (int)n;
}

int main(void) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "nq: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Window *sdl_window = SDL_CreateWindow("nq — controlled square",
                                              WIN_W, WIN_H, 0);
    if (!sdl_window) {
        fprintf(stderr, "nq: SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    NqRenderer *ren = nq_renderer_create((NqWindow *)sdl_window);
    if (!ren) {
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
        return 1;
    }

    NqInput input;
    nq_input_init(&input);

    /* SDL_Keycode for arrows + WASD are stable across SDL versions, so
     * we use them directly. (SDL_GetScancodeFromKey would also work;
     * keys.h users typically refer to keys by Keycode.) */
    SDL_Keycode KEY_LEFT  = SDLK_LEFT;
    SDL_Keycode KEY_RIGHT = SDLK_RIGHT;
    SDL_Keycode KEY_UP    = SDLK_UP;
    SDL_Keycode KEY_DOWN  = SDLK_DOWN;
    (void)KEY_LEFT; (void)KEY_RIGHT; (void)KEY_UP; (void)KEY_DOWN;

    int running = 1;
    SDL_Event event;
    Uint64 last = SDL_GetTicks();
    const Uint64 freq = SDL_GetPerformanceFrequency();
    float dt = 0.016f;  /* initial guess; replaced on first frame */

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (should_quit(&event)) {
                running = 0;
            } else {
                nq_input_pump_sdl3_event(&input, &event);
            }
        }
        nq_input_begin_frame(&input);

        /* dt via SDL_GetPerformanceCounter — same source as nq_clock
         * uses internally, so this stays consistent if we later swap
         * nq_clock in here. */
        Uint64 now = SDL_GetPerformanceCounter();
        dt = (float)((double)(now - last) / (double)freq);
        if (dt < 0.0f) dt = 0.0f;
        if (dt > 0.1f) dt = 0.1f;  /* clamp big stalls (debugger pause) */
        last = now;

        /* Input → acceleration. Held keys add to velocity each frame. */
        int ax = axis_sign(&input, SDLK_A, SDLK_D)
               + axis_sign(&input, SDLK_LEFT, SDLK_RIGHT);
        int ay = axis_sign(&input, SDLK_UP, SDLK_W)
               + axis_sign(&input, SDLK_UP, SDLK_DOWN);
        vel_x += (float)ax * ACCEL * dt;
        vel_y += (float)ay * ACCEL * dt;

        /* Friction: simple exponential decay so the box doesn't slide
         * forever after the key is released. */
        float decay = 1.0f - dt * FRICTION;
        if (decay < 0.0f) decay = 0.0f;
        vel_x *= decay;
        vel_y *= decay;

        /* Cap speed. */
        if (vel_x >  MAX_SPEED) vel_x =  MAX_SPEED;
        if (vel_x < -MAX_SPEED) vel_x = -MAX_SPEED;
        if (vel_y >  MAX_SPEED) vel_y =  MAX_SPEED;
        if (vel_y < -MAX_SPEED) vel_y = -MAX_SPEED;

        /* Integrate. */
        box_x += (int)(vel_x * dt);
        box_y += (int)(vel_y * dt);

        /* Bounce off the walls. */
        if (box_x < 0)              { box_x = 0;             vel_x = -vel_x; }
        if (box_y < 0)              { box_y = 0;             vel_y = -vel_y; }
        if (box_x + SQUARE_SIZE > WIN_W) { box_x = WIN_W - SQUARE_SIZE; vel_x = -vel_x; }
        if (box_y + SQUARE_SIZE > WIN_H) { box_y = WIN_H - SQUARE_SIZE; vel_y = -vel_y; }

        /* Render. */
        nq_renderer_clear(ren, NQ_COLOR_RGB(20, 24, 32));
        nq_renderer_fill_rect(ren,
            NQ_COLOR_RGB(220, 130, 70),
            box_x, box_y, SQUARE_SIZE, SQUARE_SIZE);

        nq_renderer_present(ren);
    }

    nq_renderer_destroy(ren);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
    return 0;
}
