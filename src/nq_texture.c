#include "nq/texture.h"
#include "nq/log.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct NqTexture {
    SDL_Texture *sdl_tex;
    SDL_Renderer *sdl_renderer;  /* kept for region offsets */
    int width;
    int height;
    char *path;  /* for logs; NULL for in-memory textures */
};

/* Try SDL3_image's IMG_Load path via weak-style fallback: if the symbol
 * isn't available at link time, we fall back to SDL3's BMP-only loader.
 * We don't drag libsdl3-image as a hard dep — engines today prefer optional
 * image loaders — so we use dlsym at first use. For now, keep static link
 * simple and try SDL_LoadBMP first; SDL_image integration is a follow-up tick. */

NqTexture *nq_texture_load(NqRenderer *renderer, const char *path) {
    if (!renderer || !path) {
        NQ_LOG_WARN("nq_texture_load: invalid args");
        return NULL;
    }
    SDL_Surface *surface = SDL_LoadBMP(path);
    if (!surface) {
        /* SDL_image not linked yet — surface load fails for non-BMP.
         * Fall through with a soft error for now; future tick adds SDL_image. */
        NQ_LOG_WARN("nq_texture_load: failed to load %s (%s) — SDL_image not linked yet?",
                    path, SDL_GetError());
        return NULL;
    }
    SDL_Texture *sdl_tex = SDL_CreateTextureFromSurface(nq_renderer_sdl(renderer), surface);
    if (!sdl_tex) {
        NQ_LOG_ERROR("nq_texture_load: SDL_CreateTextureFromSurface failed: %s",
                     SDL_GetError());
        SDL_DestroySurface(surface);
        return NULL;
    }
    NqTexture *tex = calloc(1, sizeof(NqTexture));
    if (!tex) {
        SDL_DestroyTexture(sdl_tex);
        SDL_DestroySurface(surface);
        return NULL;
    }
    tex->sdl_tex = sdl_tex;
    tex->sdl_renderer = nq_renderer_sdl(renderer);
    tex->width = surface->w;
    tex->height = surface->h;
    tex->path = strdup(path);
    SDL_DestroySurface(surface);
    NQ_LOG_INFO("nq_texture: loaded %s (%dx%d)", path, tex->width, tex->height);
    return tex;
}

NqTexture *nq_texture_load_mem(NqRenderer *renderer,
                               const void *bytes, size_t nbytes) {
    if (!renderer || !bytes || nbytes == 0) {
        NQ_LOG_WARN("nq_texture_load_mem: invalid args");
        return NULL;
    }
    /* SDL3 has no built-in mem-loader for BMP; for now this is BMP-only via
     * a temp file path workaround. Once SDL_image is linked, swap for
     * IMG_Load_IO over an SDL_IOStream. */
    NQ_LOG_WARN("nq_texture_load_mem: not implemented (BMP-only via SDL_LoadBMP_RW)" );
    (void)renderer;
    (void)bytes;
    (void)nbytes;
    return NULL;
}

void nq_texture_destroy(NqTexture *tex) {
    if (!tex) return;
    if (tex->sdl_tex) {
        SDL_DestroyTexture(tex->sdl_tex);
    }
    free(tex->path);
    free(tex);
}

int nq_texture_width(const NqTexture *tex)  { return tex ? tex->width : 0; }
int nq_texture_height(const NqTexture *tex) { return tex ? tex->height : 0; }

int nq_texture_draw(NqTexture *tex, int x, int y) {
    if (!tex || !tex->sdl_tex || !tex->sdl_renderer) return -1;
    SDL_FRect dst = { (float)x, (float)y, (float)tex->width, (float)tex->height };
    return SDL_RenderTexture(tex->sdl_renderer, tex->sdl_tex, NULL, &dst) ? -1 : 0;
}

int nq_texture_draw_region(NqTexture *tex,
                           int dst_x, int dst_y,
                           int src_x, int src_y,
                           int src_w, int src_h) {
    if (!tex || !tex->sdl_tex || !tex->sdl_renderer) return -1;
    if (src_w <= 0 || src_h <= 0) return -1;
    SDL_FRect src = { (float)src_x, (float)src_y, (float)src_w, (float)src_h };
    SDL_FRect dst = { (float)dst_x, (float)dst_y, (float)src_w, (float)src_h };
    return SDL_RenderTexture(tex->sdl_renderer, tex->sdl_tex, &src, &dst) ? -1 : 0;
}

NqTexture *nq_texture_wrap_sdl(SDL_Texture *sdl_tex, int w, int h) {
    if (!sdl_tex) return NULL;
    NqTexture *tex = calloc(1, sizeof(NqTexture));
    if (!tex) return NULL;
    tex->sdl_tex  = sdl_tex;
    tex->width    = w;
    tex->height   = h;
    tex->path     = NULL;
    return tex;
}
