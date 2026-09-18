/*
 * nq — animated_square example.
 *
 * Kitchen-sink demo: one square, bouncing off the window edges, with a
 * colour cycle driven by the action framework. Demonstrates that the
 * nq modules compose end-to-end:
 *
 *   nq_clock        — frame dt source
 *   nq_scene        — root node tree
 *   nq_node         — square's transform + callback owner
 *   nq_action_manager — owns the action graph
 *   nq_action_tween  — wraps an NqAnimFloat as an NqAction
 *   nq_animation     — float lerp primitive with ease
 *   nq_renderer_*   — drawing
 *
 * The square's RGB channels each cycle through a 3-second ease-in-out
 * loop, so the colour is always smoothly animating. Each iteration:
 * tick the clock, pump SDL events, tick the action manager, walk
 * the scene tree (update + draw), present.
 *
 * Controls: ESC or window-close to quit.
 */

#include <SDL3/SDL.h>

#include <nq/common.h>
#include <nq/graphics.h>
#include <nq/node.h>
#include <nq/scene.h>
#include <nq/clock.h>
#include <nq/animation.h>
#include <nq/action.h>
#include <nq/action_tween.h>
#include <nq/action_manager.h>

#include <stdio.h>
#include <stdlib.h>

#define WIN_W 640
#define WIN_H 480
#define SQUARE_SIZE 80
#define COLOR_PERIOD_SECONDS 3.0f

/* Last-frame dt shared between the main loop and on_each_frame — the
 * node callback signature doesn't take dt so we route it through a
 * file-scope variable. A future tick can extend the callback signature
 * with dt if this pattern becomes common. */
static float g_last_dt = 0.016f;

/* Three animations: red, green, blue — each on its own period so they
 * cycle out of phase, giving the square a continuously shifting colour.
 * Stack-allocated, lifetime matches the program. */
static NqAnimFloat red_anim;
static NqActionTween *red_tween;

static NqAnimFloat green_anim;
static NqActionTween *green_tween;

static NqAnimFloat blue_anim;
static NqActionTween *blue_tween;

static int box_x = 50;
static int box_y = 50;
static int vel_x = 140;
static int vel_y = 90;

static int should_quit(const SDL_Event *e) {
    return e->type == SDL_EVENT_QUIT ||
           (e->type == SDL_EVENT_KEY_DOWN && e->key.key == SDLK_ESCAPE);
}

static int clamp_axis(int pos, int size, int max) {
    if (pos < 0) return 0;
    if (pos + size > max) return max - size;
    return pos;
}

static void on_each_frame(NqNode *n, float dt, void *user) { (void)dt;
    (void)n;
    (void)user;
    /* The action manager's tick() drives the colour animations; this
     * node callback only owns the bounce physics. In a real game this
     * would be split across multiple nodes via the scene tree. dt is the
     * file-scope g_last_dt set from the main loop's nq_clock_tick. */
    box_x += (int)(vel_x * g_last_dt);
    box_y += (int)(vel_y * g_last_dt);

    if (box_x <= 0 || box_x + SQUARE_SIZE >= WIN_W) {
        vel_x = -vel_x;
        box_x = clamp_axis(box_x, SQUARE_SIZE, WIN_W);
    }
    if (box_y <= 0 || box_y + SQUARE_SIZE >= WIN_H) {
        vel_y = -vel_y;
        box_y = clamp_axis(box_y, SQUARE_SIZE, WIN_H);
    }
}

static void draw_each_frame(NqNode *n, void *user) {
    (void)n;
    NqRenderer *ren = (NqRenderer *)user;
    /* Compose colour from the three independent NqAnimFloats. Each one
     * runs its own ease curve so the resulting RGB is non-trivial. */
    NqColor c = NQ_COLOR_RGBA(
        (uint8_t)nq_anim_float_value(&red_anim),
        (uint8_t)nq_anim_float_value(&green_anim),
        (uint8_t)nq_anim_float_value(&blue_anim),
        255
    );
    nq_renderer_clear(ren, NQ_COLOR_RGB(20, 24, 32));
    nq_renderer_fill_rect(ren, c, box_x, box_y, SQUARE_SIZE, SQUARE_SIZE);
}

int main(void) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "nq: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Window *sdl_window = SDL_CreateWindow("nq — animated square",
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

    /* Engine state: clock + scene + action manager. */
    NqClock *clock = nq_clock_create();
    NqScene  *scene = nq_scene_create();
    NqActionManager *am = nq_action_manager_create();

    /* Root node owns the per-frame draw + update callbacks. */
    nq_node_set_callbacks(nq_scene_root(scene),
                           NULL, on_each_frame, draw_each_frame, ren);

    /* Three RGB animators on the action manager — each cycles 0..255 over
     * 3 seconds. Different `from`/`to` ranges keep them out of phase. */
    nq_anim_float_init(&red_anim,    60.0f, 220.0f, COLOR_PERIOD_SECONDS, NQ_EASE_SINE_IN_OUT);
    nq_anim_float_init(&green_anim, 200.0f,  60.0f, COLOR_PERIOD_SECONDS, NQ_EASE_SINE_IN_OUT);
    nq_anim_float_init(&blue_anim,  140.0f, 230.0f, COLOR_PERIOD_SECONDS, NQ_EASE_SINE_IN_OUT);

    red_tween   = nq_action_tween_create(&red_anim);
    green_tween = nq_action_tween_create(&green_anim);
    blue_tween  = nq_action_tween_create(&blue_anim);
    nq_action_manager_add(am, nq_action_tween_action(red_tween));
    nq_action_manager_add(am, nq_action_tween_action(green_tween));
    nq_action_manager_add(am, nq_action_tween_action(blue_tween));

    /* Main loop. */
    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (should_quit(&event)) running = 0;
        }
        NqFrameTime ft;
        nq_clock_tick(clock, &ft);
        g_last_dt = ft.delta_seconds;
        nq_action_manager_tick(am, ft.delta_seconds);
        nq_node_update(nq_scene_root(scene), ft.delta_seconds);
        nq_node_draw(nq_scene_root(scene));
        nq_renderer_present(ren);
    }

    /* Teardown — actions owned by caller; manager / scene / clock are
     * nq-owned and need their destroy calls. */
    nq_action_tween_destroy(red_tween);
    nq_action_tween_destroy(green_tween);
    nq_action_tween_destroy(blue_tween);
    nq_action_manager_destroy(am);
    nq_scene_destroy(scene);
    nq_clock_destroy(clock);
    nq_renderer_destroy(ren);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
    return 0;
}
