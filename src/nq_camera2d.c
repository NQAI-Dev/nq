#include <math.h>

#include <nq/camera2d.h>

static NqVec2f nq_camera2d_viewport_center(const NqCamera2D *camera) {
    return nq_vec2f(
        (float)camera->viewport.x + (float)camera->viewport.w * 0.5f,
        (float)camera->viewport.y + (float)camera->viewport.h * 0.5f
    );
}

void nq_camera2d_init(NqCamera2D *camera, NqVec2f center, NqRect viewport) {
    if (!camera) return;
    camera->center = center;
    camera->viewport = viewport;
    camera->zoom = 1.0f;
}

bool nq_camera2d_set_zoom(NqCamera2D *camera, float zoom) {
    if (!camera || !isfinite(zoom) || zoom <= 0.0f) return false;
    camera->zoom = zoom;
    return true;
}

void nq_camera2d_pan(NqCamera2D *camera, NqVec2f delta) {
    if (!camera) return;
    camera->center = nq_vec2f_add(camera->center, delta);
}

NqVec2f nq_camera2d_world_to_screen(const NqCamera2D *camera, NqVec2f world) {
    if (!camera || !isfinite(camera->zoom) || camera->zoom <= 0.0f) {
        return nq_vec2f(0.0f, 0.0f);
    }
    NqVec2f viewport_center = nq_camera2d_viewport_center(camera);
    return nq_vec2f(
        viewport_center.x + (world.x - camera->center.x) * camera->zoom,
        viewport_center.y + (world.y - camera->center.y) * camera->zoom
    );
}

NqVec2f nq_camera2d_screen_to_world(const NqCamera2D *camera, NqVec2f screen) {
    if (!camera || !isfinite(camera->zoom) || camera->zoom <= 0.0f) {
        return nq_vec2f(0.0f, 0.0f);
    }
    NqVec2f viewport_center = nq_camera2d_viewport_center(camera);
    return nq_vec2f(
        camera->center.x + (screen.x - viewport_center.x) / camera->zoom,
        camera->center.y + (screen.y - viewport_center.y) / camera->zoom
    );
}

bool nq_camera2d_zoom_at(NqCamera2D *camera, float zoom, NqVec2f screen_anchor) {
    if (!camera || !isfinite(zoom) || zoom <= 0.0f ||
        !isfinite(camera->zoom) || camera->zoom <= 0.0f) {
        return false;
    }
    NqVec2f world_anchor = nq_camera2d_screen_to_world(camera, screen_anchor);
    camera->zoom = zoom;
    NqVec2f world_after = nq_camera2d_screen_to_world(camera, screen_anchor);
    camera->center = nq_vec2f_add(camera->center,
                                  nq_vec2f_sub(world_anchor, world_after));
    return true;
}
