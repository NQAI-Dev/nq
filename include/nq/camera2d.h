/*
 * nq — backend-independent 2D camera transforms.
 *
 * The camera centre is expressed in world coordinates. The viewport is in
 * screen pixels, and zoom is screen pixels per world unit. All transforms use
 * floats so callers decide how and when to round for raster rendering.
 */
#ifndef NQ_CAMERA2D_H
#define NQ_CAMERA2D_H

#include <stdbool.h>

#include <nq/rect.h>
#include <nq/vec.h>

typedef struct {
    NqVec2f center;
    NqRect viewport;
    float zoom;
} NqCamera2D;

/* Initialise a camera at `center`, with 1:1 world-to-screen scale. */
void nq_camera2d_init(NqCamera2D *camera, NqVec2f center, NqRect viewport);

/* Set a finite, positive zoom. Invalid values leave the camera unchanged. */
bool nq_camera2d_set_zoom(NqCamera2D *camera, float zoom);

/* Translate the camera in world units. A positive X pan moves the world left. */
void nq_camera2d_pan(NqCamera2D *camera, NqVec2f delta);

/* Convert between world coordinates and viewport-relative screen pixels. */
NqVec2f nq_camera2d_world_to_screen(const NqCamera2D *camera, NqVec2f world);
NqVec2f nq_camera2d_screen_to_world(const NqCamera2D *camera, NqVec2f screen);

/*
 * Change zoom while keeping the world point under `screen_anchor` fixed.
 * Useful for cursor-centred wheel zoom. Invalid zoom values return false and
 * leave both centre and zoom unchanged.
 */
bool nq_camera2d_zoom_at(NqCamera2D *camera, float zoom, NqVec2f screen_anchor);

#endif /* NQ_CAMERA2D_H */
