/*
 * nq — mouse_paint example.
 *
 * A click-and-drag paint app demonstrating the full Phase 7 input API:
 *   - nq_input_mouse_inside_rect()  → hover detection (panel buttons)
 *   - nq_input_mouse_pressed()      → edge-trigger (clear button click)
 *   - nq_input_mouse_down()         → level-trigger (drag-paint)
 *   - nq_input_mouse_wheel()        → brush size adjustment
 *   - nq_fps_counter_*()            → FPS overlay
 *
 * Differences from controlled_square: this example consumes mouse
 * (not keyboard), exercises the entire mouse primitive set, and
 * shows the FPS counter integrated into a real render loop.
 */

#include <SDL3/SDL.h>

#include <nq/common.h>
#include <nq/graphics.h>
#include <nq/input.h>
#include <nq/input_sdl3.h>
#include <nq/fps.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Canvas grid: 32 cols × 24 rows × 20px = 640×480 (matches window). */
#define WIN_W         740
#define WIN_H         480
#define GRID_COLS     32
#define GRID_ROWS     24
#define CELL          20
#define PANEL_X       (GRID_COLS * CELL)
#define PANEL_W       (WIN_W - PANEL_X)

/* Panel widgets (right side). */
#define BTN_CLEAR_X   (PANEL_X + 10)
#define BTN_CLEAR_Y   10
#define BTN_CLEAR_W   80
#define BTN_CLEAR_H   40

#define LABEL_BRUSHY  80
#define LABEL_FPSY    130

/* Brush radius in cells (1..3). Wheel up/down adjusts. */
#define BRUSH_MIN     1
#define BRUSH_MAX     3

static int canvas[GRID_ROWS][GRID_COLS];
static int brush_size = 1;

static int should_quit(const SDL_Event *e) {
    return e->type == SDL_EVENT_QUIT
        || (e->type == SDL_EVENT_KEY_DOWN && e->key.key == SDLK_ESCAPE);
}

/* Paint a single cell if it falls inside the canvas. */
static void paint_cell(int gx, int gy) {
    if (gx >= 0 && gx < GRID_COLS && gy >= 0 && gy < GRID_ROWS) {
        canvas[gy][gx] = 1;
    }
}

/* Paint a circular brush centred on (cx, cy). Uses brush_size as
 * the radius in cells; cells whose centre is within radius² are
 * marked painted. Stamps the brush once per frame so drag-paint
 * looks continuous. */
static void paint_brush(int cx, int cy) {
    int r2 = brush_size * brush_size;
    for (int dy = -brush_size + 1; dy < brush_size; dy++) {
        for (int dx = -brush_size + 1; dx < brush_size; dx++) {
            if (dx * dx + dy * dy <= r2) {
                paint_cell(cx + dx, cy + dy);
            }
        }
    }
}

/* Wheel up → bigger brush, down → smaller. Clamp to [BRUSH_MIN, BRUSH_MAX]. */
static void update_brush_from_wheel(int wheel) {
    while (wheel > 0 && brush_size < BRUSH_MAX) { brush_size++; wheel--; }
    while (wheel < 0 && brush_size > BRUSH_MIN) { brush_size--; wheel++; }
}

int main(void) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "nq: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *sdl_window = SDL_CreateWindow("nq mouse_paint", WIN_W, WIN_H, 0);
    if (!sdl_window) {
        fprintf(stderr, "nq: SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    NqWindow *window = nq_window_wrap_sdl(sdl_window);
    NqRenderer *ren = nq_renderer_create(window);
    if (!ren) {
        fprintf(stderr, "nq: nq_renderer_create failed\n");
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
        return 1;
    }

    NqInput input;
    nq_input_init(&input);

    NqFpsCounter fps;
    nq_fps_counter_init(&fps);

    int running = 1;
    SDL_Event event;

    /* SDL_GetPerformanceCounter is the same source nq_clock uses,
     * so the elapsed_seconds we feed the FPS counter stays
     * consistent if we swap nq_clock in here later. */
    Uint64 last = SDL_GetPerformanceCounter();
    const Uint64 freq = SDL_GetPerformanceFrequency();

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
        double elapsed = (double)now / (double)freq;
        nq_fps_counter_tick(&fps, elapsed);
        last = now;

        int mx = nq_input_mouse_x(&input);
        int my = nq_input_mouse_y(&input);
        int cell_x = mx / CELL;
        int cell_y = my / CELL;
        int in_canvas = (mx >= 0 && mx < GRID_COLS * CELL
                      && my >= 0 && my < GRID_ROWS * CELL);

        /* Wheel → brush size (consume once per frame). */
        int wheel = nq_input_mouse_wheel(&input);
        if (wheel != 0) update_brush_from_wheel(wheel);

        /* Clear button (edge-triggered — only fires on the frame
         * the click started, not while held). */
        if (nq_input_mouse_pressed(&input, NQ_MOUSE_BUTTON_LEFT)
            && nq_input_mouse_inside_rect(&input,
                BTN_CLEAR_X, BTN_CLEAR_Y, BTN_CLEAR_W, BTN_CLEAR_H)) {
            memset(canvas, 0, sizeof(canvas));
        }

        /* Drag-paint inside canvas (level-triggered — keeps painting
         * every frame the button is held and the cursor moves). */
        if (in_canvas && nq_input_mouse_down(&input, NQ_MOUSE_BUTTON_LEFT)) {
            paint_brush(cell_x, cell_y);
        }

        /* ---------- Render ---------- */
        nq_renderer_clear(ren, NQ_COLOR_RGB(20, 24, 32));

        /* Painted cells (red). */
        for (int y = 0; y < GRID_ROWS; y++) {
            for (int x = 0; x < GRID_COLS; x++) {
                if (canvas[y][x]) {
                    nq_renderer_fill_rect(ren, NQ_COLOR_RGB(220, 70, 70),
                        x * CELL, y * CELL, CELL, CELL);
                }
            }
        }

        /* Hover cell highlight (yellow outline only — we use a
         * semi-transparent-ish mix by drawing a smaller rect over
         * the full cell; this works with the opaque NqColor API). */
        if (in_canvas) {
            /* Inner cell is painted yellow if empty, otherwise the
             * existing red shows through the cursor outline (drawn
             * as 2px-thick border via 4 thin rects). */
            int hx = cell_x * CELL;
            int hy = cell_y * CELL;
            NqColor border = NQ_COLOR_RGB(255, 220, 100);
            nq_renderer_fill_rect(ren, border, hx, hy, CELL, 2);          /* top    */
            nq_renderer_fill_rect(ren, border, hx, hy + CELL - 2, CELL, 2);/* bottom */
            nq_renderer_fill_rect(ren, border, hx, hy, 2, CELL);          /* left   */
            nq_renderer_fill_rect(ren, border, hx + CELL - 2, hy, 2, CELL);/* right  */
        }

        /* Right panel background. */
        nq_renderer_fill_rect(ren, NQ_COLOR_RGB(40, 44, 52),
            PANEL_X, 0, PANEL_W, WIN_H);

        /* Clear button (red rectangle). */
        nq_renderer_fill_rect(ren, NQ_COLOR_RGB(180, 60, 60),
            BTN_CLEAR_X, BTN_CLEAR_Y, BTN_CLEAR_W, BTN_CLEAR_H);

        /* Brush size indicator (1/2/3 filled squares in the panel).
         * Shows the current brush_size by drawing that many squares
         * stacked horizontally; remaining squares are dim. */
        for (int i = 0; i < BRUSH_MAX; i++) {
            int x = PANEL_X + 15 + i * 24;
            NqColor c = (i < brush_size)
                ? NQ_COLOR_RGB(120, 200, 255)
                : NQ_COLOR_RGB(70, 80, 95);
            nq_renderer_fill_rect(ren, c, x, LABEL_BRUSHY, 20, 20);
        }

        /* FPS overlay — drawn as a row of small filled squares (1 per
         * 10 fps), so the user sees FPS at a glance without needing a
         * text renderer. nq's renderer has no text API yet. */
        int fps = nq_fps_counter_get(&fps);
        int bars = (fps + 5) / 10;  /* round to nearest 10 */
        if (bars > 12) bars = 12;
        for (int i = 0; i < bars; i++) {
            NqColor c = (i < 6)  ? NQ_COLOR_RGB(80, 220, 110)
                      : (i < 10) ? NQ_COLOR_RGB(230, 200, 80)
                                 : NQ_COLOR_RGB(230, 100, 80);
            nq_renderer_fill_rect(ren, c,
                PANEL_X + 10 + i * 6, LABEL_FPSY, 4, 12);
        }

        nq_renderer_present(ren);
    }

    nq_renderer_destroy(ren);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
    return 0;
}
