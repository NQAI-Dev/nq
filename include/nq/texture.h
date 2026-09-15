/*
 * nq — texture wrapper over SDL3_Texture.
 *
 * Loads images from disk (BMP/PNG/JPG via SDL3_image if available at
 * runtime; otherwise we fall back to SDL3's built-in BMP loader). Stores
 * (w,h) at load so consumers don't need a separate query.
 *
 * The texture is bound to a renderer for its lifetime — destroying the
 * renderer out-of-order is undefined. For now: 1 texture : 1 renderer.
 */
#ifndef NQ_TEXTURE_H
#define NQ_TEXTURE_H

#include <stddef.h>
#include <stdint.h>
#include "nq/graphics.h"  /* NqRenderer */

/* Forward declaration so consumers can reference SDL_Texture* without
 * pulling in <SDL3/SDL.h> for the typedef. Same pattern as the
 * SDL_Renderer forward-decl in nq/graphics.h (d036c99). */
typedef struct SDL_Texture SDL_Texture;

typedef struct NqTexture NqTexture;

NqTexture *nq_texture_load(NqRenderer *renderer, const char *path);
NqTexture *nq_texture_load_mem(NqRenderer *renderer,
                               const void *bytes, size_t nbytes);
void       nq_texture_destroy(NqTexture *tex);
int        nq_texture_width(const NqTexture *tex);
int        nq_texture_height(const NqTexture *tex);
/* Draw texture at (x,y) covering its full size. */
int        nq_texture_draw(NqTexture *tex, int x, int y);
/* Draw a sub-rectangle of the texture. */
int        nq_texture_draw_region(NqTexture *tex,
                                  int dst_x, int dst_y,
                                  int src_x, int src_y,
                                  int src_w, int src_h);


/* Wrap an existing SDL_Texture* into an NqTexture. For tools / tests /
 * examples that build the underlying texture procedurally rather than
 * loading it from disk. Caller transfers ownership of the SDL_Texture
 * to the returned NqTexture (nq_texture_destroy will free it). */
NqTexture *nq_texture_wrap_sdl(SDL_Texture *sdl_tex, int w, int h);

#endif /* NQ_TEXTURE_H */
