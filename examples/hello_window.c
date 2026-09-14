/*
 * nq — first runnable example.
 *
 * SDL3 hello window: opens a 640x480 window, clears to cornflower blue
 * each frame, exits on ESC or window close. This is the smallest possible
 * "engine" — proves the SDL3 dep is wired and the CMake pipeline works.
 * The actual nq_graphics_* API will replace the SDL calls as the engine
 * grows; for now everything is inlined so the first commit has zero
 * abstraction overhead and is trivially auditable.
 */

#include <SDL3/SDL.h>
#include <stdio.h>

#define NQ_WINDOW_W 640
#define NQ_WINDOW_H 480
#define NQ_WINDOW_TITLE "nq — first window"

static int nq_should_quit(SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) {
        return 1;
    }
    if (event->type == SDL_EVENT_KEY_DOWN &&
        event->key.key == SDLK_ESCAPE) {
        return 1;
    }
    return 0;
}

int main(void) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "nq: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        NQ_WINDOW_TITLE, NQ_WINDOW_W, NQ_WINDOW_H, 0);
    if (!window) {
        fprintf(stderr, "nq: SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        fprintf(stderr, "nq: SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (nq_should_quit(&event)) {
                running = 0;
            }
        }
        /* Cornflower blue clear each frame — proves the swap chain is
         * actually pushing pixels. Replace with a draw call once the
         * nq_graphics_clear(renderer, color) API exists. */
        SDL_SetRenderDrawColor(renderer, 100, 149, 237, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
