#include "nq/camera_utils.h"
#include <math.h>

NqRect nq_camera2d_get_viewport_rect(const NqCamera2D *camera) {
    if (!camera || camera->zoom <= 0.0f) {
        NqRect r = {0, 0, 0, 0};
        return r;
    }
    
    // Top-left of the screen in world coordinates
    NqVec2f tl_screen = { (float)camera->viewport.x, (float)camera->viewport.y };
    NqVec2f tl = nq_camera2d_screen_to_world(camera, tl_screen);
    
    // Bottom-right of the screen in world coordinates
    NqVec2f br_screen = { (float)(camera->viewport.x + camera->viewport.w), (float)(camera->viewport.y + camera->viewport.h) };
    NqVec2f br = nq_camera2d_screen_to_world(camera, br_screen);
    
    NqRect r;
    r.x = (int)floorf(tl.x);
    r.y = (int)floorf(tl.y);
    r.w = (int)ceilf(br.x - tl.x);
    r.h = (int)ceilf(br.y - tl.y);
    return r;
}

void nq_camera2d_clamp_to_bounds(NqCamera2D *camera, NqRect world_bounds) {
    if (!camera || camera->zoom <= 0.0f) return;
    
    float view_w = (float)camera->viewport.w / camera->zoom;
    float view_h = (float)camera->viewport.h / camera->zoom;
    
    float min_x = (float)world_bounds.x + view_w * 0.5f;
    float max_x = (float)(world_bounds.x + world_bounds.w) - view_w * 0.5f;
    float min_y = (float)world_bounds.y + view_h * 0.5f;
    float max_y = (float)(world_bounds.y + world_bounds.h) - view_h * 0.5f;
    
    if (min_x > max_x) {
        camera->center.x = (float)world_bounds.x + (float)world_bounds.w * 0.5f;
    } else {
        if (camera->center.x < min_x) camera->center.x = min_x;
        if (camera->center.x > max_x) camera->center.x = max_x;
    }
    
    if (min_y > max_y) {
        camera->center.y = (float)world_bounds.y + (float)world_bounds.h * 0.5f;
    } else {
        if (camera->center.y < min_y) camera->center.y = min_y;
        if (camera->center.y > max_y) camera->center.y = max_y;
    }
}

void nq_camera2d_drag_pan(NqCamera2D *camera, const NqInput *input, bool is_dragging) {
    if (!camera || !input || camera->zoom <= 0.0f || !is_dragging) return;
    
    float dx = (float)nq_input_mouse_dx(input);
    float dy = (float)nq_input_mouse_dy(input);
    
    NqVec2f pan_delta = { -dx / camera->zoom, -dy / camera->zoom };
    nq_camera2d_pan(camera, pan_delta);
}
