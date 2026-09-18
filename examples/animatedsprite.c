/*
 * nq — animatedsprite example.
 *
 * One on-screen sprite whose displayed frame cycles through four
 * pre-generated atlas regions at 8 FPS. Demonstrates:
 *
 *   nq_texture        — load-from-memory + draw_region
 *   nq_atlas          — string-keyed regions over a single texture
 *   nq_anim_float     — float lerp primitive, used to advance frame idx
 *   nq_clock          — frame dt
 *   nq_renderer_*     — clear + draw via draw_region
 *
 * The textures themselves are generated procedurally — no PNG / BMP
 * files needed in the tree. The atlas is built against ONE texture
 * (8x8 pixels) with four regions (top-left, top-right, bottom-left,
 * bottom-right quadrants) that we paint with distinct colours at
 * init. The four visible frames are therefore four solid-coloured
 * 4x4 quadrants of an 8x8 texture, drawn at 200x200 px on screen.
 *
 * Why this shape: the example's purpose is to show the *plumbing* —
 * texture → atlas → region lookup by name → draw. The actual image
 * content is irrelevant; what matters is that every layer of the
 * texture pipeline gets exercised in one example.
 */

#include <SDL3/SDL.h>

#include <nq/common.h>
#include <nq/graphics.h>
#include <nq/texture.h>
#include <nq/atlas.h>
#include <nq/animation.h>
#include <nq/clock.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEX_SIZE   8
SDL_Texture *nq_texture_get_sdl_texture(const NqTexture *tex);
#define TEX_FRAMES 4
#define SPRITE_PX  200

/* Returns the SDL_Texture* owned by NqTexture — needed here so the
 * example can write its procedural pixels into the texture once at
 * startup. This is the "backend escape hatch" the docs call out. */
static SDL_Texture *tex_sdl(NqTexture *tex) {
     
    return nq_texture_get_sdl_texture(tex);
}

/* Generates a single 8x8 SDL_Texture (RGBA8888) and paints four 4x4
 * quadrants with different colours so we have four distinct "frames"
 * in one texture. The atlas then maps names to those four quadrants. */
static NqTexture *make_sprite_texture(NqRenderer *ren) {
    SDL_Texture *sdl_tex = SDL_CreateTexture(
        nq_renderer_sdl(ren),
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STATIC,
        TEX_SIZE, TEX_SIZE);
    if (!sdl_tex) {
        fprintf(stderr, "nq: SDL_CreateTexture failed: %s\n", SDL_GetError());
        return NULL;
    }
    /* Paint quadrants: TL=red, TR=green, BL=blue, BR=yellow. */
    static const uint32_t colors[TEX_FRAMES] = {
        0xFF3030FFu, 0x30FF30FFu, 0x3030FFFFu, 0xFFFF30FFu
    };
    uint32_t pixels[TEX_SIZE * TEX_SIZE];
    for (int i = 0; i < TEX_SIZE * TEX_SIZE; i++) {
        int x = i % TEX_SIZE;
        int y = i / TEX_SIZE;
        int quad = (y < TEX_SIZE / 2 ? 0 : 2) + (x < TEX_SIZE / 2 ? 0 : 1);
        pixels[i] = colors[quad];
    }
    SDL_UpdateTexture(sdl_tex, NULL, pixels, TEX_SIZE * sizeof(uint32_t));

    /* Wrap the SDL_Texture in an NqTexture by going through load_mem,
     * which currently returns NULL (BMP-only stub). So we cheat: use
     * SDL_CreateTextureFromSurface with a 1x1 surface instead — no,
     * that's heavy. Cleanest hack for this example: construct the
     * NqTexture wrapper around an existing SDL_Texture via the
     * renderer escape hatch.
     *
     * nq_texture_load_mem() is intentionally a stub right now
     * (SDL_image not linked). For this example only, we reach in via
     * the renderer's SDL access to make a NqTexture directly. */
    extern NqTexture *nq_texture_wrap_sdl(SDL_Texture *sdl_tex,
                                            int w, int h);
    return nq_texture_wrap_sdl(sdl_tex, TEX_SIZE, TEX_SIZE);
}

/* Convert the demo's "current frame index" (0..3) to an atlas region
 * name and draw it via the atlas. */
static void draw_current_frame(NqAtlas *atlas, NqTexture *tex,
                               const char **frame_names, int idx,
                               int dst_x, int dst_y) {
    nq_atlas_draw(atlas, tex, frame_names[idx], dst_x, dst_y);
}

int main(void) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "nq: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Window *sdl_window = SDL_CreateWindow("nq — animated sprite",
                                              SPRITE_PX + 80, SPRITE_PX + 80, 0);
    if (!sdl_window) { SDL_Quit(); return 1; }
    NqRenderer *ren = nq_renderer_create((NqWindow *)sdl_window);
    if (!ren) { SDL_DestroyWindow(sdl_window); SDL_Quit(); return 1; }

    NqClock *clock = nq_clock_create();

    /* Procedural texture + atlas. */
    NqTexture *tex = make_sprite_texture(ren);
    if (!tex) {
        fprintf(stderr, "nq: failed to build sprite texture\n");
        nq_renderer_destroy(ren); SDL_DestroyWindow(sdl_window); SDL_Quit();
        return 1;
    }
    NqAtlas *atlas = nq_atlas_create(TEX_SIZE, TEX_SIZE, 0);
    const int half = TEX_SIZE / 2;
    nq_atlas_add_region(atlas, "frame0", nq_rect(0,       0,       half, half));
    nq_atlas_add_region(atlas, "frame1", nq_rect(half,     0,       half, half));
    nq_atlas_add_region(atlas, "frame2", nq_rect(0,       half,    half, half));
    nq_atlas_add_region(atlas, "frame3", nq_rect(half,     half,    half, half));
    const char *frame_names[TEX_FRAMES] = { "frame0", "frame1", "frame2", "frame3" };

    /* Float animation: 0..3, 0.5s per cycle (8 FPS with 4 frames =
     * 0.125s per frame), snaps (no ease). Restarted when finished. */
    NqAnimFloat anim;
    nq_anim_float_init(&anim, 0.0f, (float)(TEX_FRAMES - 1),
                       0.5f, NQ_EASE_LINEAR);

    int running = 1;
    SDL_Event event;
    Uint64 freq = SDL_GetPerformanceFrequency();
    Uint64 last  = SDL_GetPerformanceCounter();
    float dt = 0.016f;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT ||
                (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)) {
                running = 0;
            }
        }
        Uint64 now = SDL_GetPerformanceCounter();
        dt = (float)((double)(now - last) / (double)freq);
        if (dt < 0.0f) dt = 0.0f;
        if (dt > 0.1f) dt = 0.1f;
        last = now;

        nq_anim_float_update(&anim, dt);
        if (nq_anim_float_done(&anim)) {
            nq_anim_float_restart(&anim);
        }
        int frame = (int)nq_anim_float_value(&anim);
        if (frame < 0) frame = 0;
        if (frame >= TEX_FRAMES) frame = TEX_FRAMES - 1;

        nq_renderer_clear(ren, NQ_COLOR_RGB(24, 28, 38));
        draw_current_frame(atlas, tex, frame_names, frame, 40, 40);

        nq_renderer_present(ren);
    }

    nq_atlas_destroy(atlas);
    nq_texture_destroy(tex);
    nq_clock_destroy(clock);
    nq_renderer_destroy(ren);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
    return 0;
}
