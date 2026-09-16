/*
 * nq — action_pipeline example.
 *
 * A square that moves across the screen under the control of the
 * action framework. Demonstrates the full animation pipeline:
 *
 *   NqAnimVec2      — per-frame interpolation
 *   nq_anim_vec2_*  — ease curve + lerp primitive
 *   nq_action_tween_vec2  — bridges the animation into the action
 *   NqActionManager — owns and ticks all active actions each frame
 *   nq_renderer_*   — clear + fill_rect
 *
 * On startup the action is scheduled (square moves from left edge
 * to right edge over 2 seconds). After 2 seconds the action finishes
 * itself; the manager prunes it on the next tick. The example then
 * restarts the animation, looping the square across the screen.
 *
 * Controls: ESC quits.
 */

#include <SDL3/SDL.h>

#include <nq/common.h>
#include <nq/graphics.h>
#include <nq/animation.h>
#include <nq/anim_vec2.h>
#include <nq/action.h>
#include <nq/action_tween_vec2.h>
#include <nq/action_manager.h>

#include <stdio.h>
#include <stdlib.h>

#define WIN_W 640
#define WIN_H 240
#define BOX_SIZE 60
#define MOVE_DURATION_SECONDS 2.0f

typedef struct {
    int ticks;
    int target_ticks;
} MoveCtx;

/* User tick: increments counter each frame; returns FINISHED when
 * the animation duration in ticks is exceeded. The action wrapper
 * routes this tick callback each frame the wrapper is RUNNING. */
static NqActionState move_tick(NqAction *a, float dt, void *user) {
    (void)a;
    MoveCtx *c = (MoveCtx *)user;
    if (!c) return NQ_ACTION_FINISHED;
    c->ticks++;
    /* Duration in ticks: 60 fps × duration seconds. */
    int target = (int)(60.0f * MOVE_DURATION_SECONDS + 0.5f);
    if (c->ticks >= target) return NQ_ACTION_FINISHED;
    return NQ_ACTION_RUNNING;
}

static int should_quit(const SDL_Event *e) {
    return e->type == SDL_EVENT_QUIT ||
           (e->type == SDL_EVENT_KEY_DOWN && e->key.key == SDLK_ESCAPE);
}

int main(void) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "nq: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Window *sdl_window = SDL_CreateWindow("nq — action pipeline",
                                              WIN_W, WIN_H, 0);
    if (!sdl_window) { SDL_Quit(); return 1; }

    NqRenderer *ren = nq_renderer_create((NqWindow *)sdl_window);
    if (!ren) { SDL_DestroyWindow(sdl_window); SDL_Quit(); return 1; }

    /* Engine state. */
    NqActionManager *am = nq_action_manager_create();

    /* Animation that drives the box position. The box moves from
     * (40, 60) to (WIN_W - 40 - BOX_SIZE, 60) over MOVE_DURATION_SECONDS. */
    NqAnimVec2 anim;
    nq_anim_vec2_init(&anim,
        nq_vec2f(40.0f, 60.0f),
        nq_vec2f((float)(WIN_W - 40 - BOX_SIZE), 60.0f),
        MOVE_DURATION_SECONDS, NQ_EASE_QUAD_IN_OUT);

    MoveCtx move_ctx = {0, 0};
    NqActionTweenVec2 *wrapper =
        nq_action_tween_vec2_create(move_tick, &move_ctx);
    if (!wrapper) {
        nq_action_manager_destroy(am);
        nq_renderer_destroy(ren);
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
        return 1;
    }
    nq_action_manager_add(am, nq_action_tween_vec2_action(wrapper));

    int running = 1;
    SDL_Event event;
    Uint64 freq = SDL_GetPerformanceFrequency();
    Uint64 last = SDL_GetPerformanceCounter();
    float dt = 0.016f;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (should_quit(&event)) running = 0;
        }

        Uint64 now = SDL_GetPerformanceCounter();
        dt = (float)((double)(now - last) / (double)freq);
        if (dt < 0.0f) dt = 0.0f;
        if (dt > 0.1f) dt = 0.1f;
        last = now;

        /* Tick the action manager: drives the wrapper, which drives
         * move_tick, which advances our counter. */
        nq_action_manager_tick(am, dt);

        /* Read current position from the animation and render. */
        NqVec2f pos = nq_anim_vec2_value(&anim);
        nq_renderer_clear(ren, NQ_COLOR_RGB(20, 24, 32));
        nq_renderer_fill_rect(ren,
            NQ_COLOR_RGB(240, 160, 60),
            (int)pos.x, (int)pos.y, BOX_SIZE, BOX_SIZE);
        nq_renderer_present(ren);

        /* When the action finishes, restart it for a continuous loop. */
        if (nq_anim_vec2_done(&anim)) {
            nq_anim_vec2_restart(&anim);
            move_ctx.ticks = 0;
            nq_action_manager_add(am, nq_action_tween_vec2_action(wrapper));
        }
    }

    nq_action_tween_vec2_destroy(wrapper);
    nq_action_manager_destroy(am);
    nq_renderer_destroy(ren);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
    return 0;
}
