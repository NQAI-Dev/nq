#ifndef NQ_CAMERA_UTILS_H
#define NQ_CAMERA_UTILS_H

#include "nq/camera2d.h"
#include "nq/input.h"
#include "nq/rect.h"

/* Returns a bounding box around the visible area of the camera. */
NqRect nq_camera2d_get_viewport_rect(const NqCamera2D *camera);

/* Updates camera position to enforce that its viewport stays entirely within the world bounds. */
void nq_camera2d_clamp_to_bounds(NqCamera2D *camera, NqRect world_bounds);

/* Drags the camera smoothly based on mouse input. Uses mouse delta from NqInput. */
void nq_camera2d_drag_pan(NqCamera2D *camera, const NqInput *input, bool is_dragging);

#endif
