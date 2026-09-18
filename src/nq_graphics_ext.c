#include "nq/graphics_ext.h"

int nq_renderer_fill_round_rect(NqRenderer *r, NqColor color, NqRect rect, int radius) {
    if (!r) return -1;
    if (radius <= 0) return nq_renderer_fill_rect(r, color, rect.x, rect.y, rect.w, rect.h);
    
    // clamp radius
    if (radius > rect.w / 2) radius = rect.w / 2;
    if (radius > rect.h / 2) radius = rect.h / 2;

    // Center cross
    nq_renderer_fill_rect(r, color, rect.x + radius, rect.y, rect.w - 2 * radius, rect.h);
    nq_renderer_fill_rect(r, color, rect.x, rect.y + radius, radius, rect.h - 2 * radius);
    nq_renderer_fill_rect(r, color, rect.x + rect.w - radius, rect.y + radius, radius, rect.h - 2 * radius);

    // 4 corners (drawing a quarter circle in each using fill_circle)
    // SDL3 primitive rendering doesn't easily support arcs, so we cheat with circle overlapping/clipping
    // The proper way would be writing a custom circle quadrant rasterizer, but we use simple filled circles
    // masked/overlaid. For a lightweight C framework, generating geometry is the right path later.
    // Instead we'll implement a custom quadrant rasterizer inline.
    
    int cx, cy;
    int x = radius - 1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    nq_renderer_set_draw_color(r, color);

    while (x >= y) {
        // TL quadrant
        cx = rect.x + radius; cy = rect.y + radius;
        nq_renderer_draw_line(r, color, cx - x, cy - y, cx, cy - y);
        nq_renderer_draw_line(r, color, cx - y, cy - x, cx, cy - x);
        
        // TR quadrant
        cx = rect.x + rect.w - radius - 1; cy = rect.y + radius;
        nq_renderer_draw_line(r, color, cx, cy - y, cx + x, cy - y);
        nq_renderer_draw_line(r, color, cx, cy - x, cx + y, cy - x);

        // BL quadrant
        cx = rect.x + radius; cy = rect.y + rect.h - radius - 1;
        nq_renderer_draw_line(r, color, cx - x, cy + y, cx, cy + y);
        nq_renderer_draw_line(r, color, cx - y, cy + x, cx, cy + x);

        // BR quadrant
        cx = rect.x + rect.w - radius - 1; cy = rect.y + rect.h - radius - 1;
        nq_renderer_draw_line(r, color, cx, cy + y, cx + x, cy + y);
        nq_renderer_draw_line(r, color, cx, cy + x, cx + y, cy + x);

        if (err <= 0) {
            y++;
            err += dy;
            dy += 2;
        }
        if (err > 0) {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
    return 0;
}

int nq_renderer_draw_round_rect(NqRenderer *r, NqColor color, NqRect rect, int radius) {
    if (!r) return -1;
    if (radius <= 0) return nq_renderer_draw_rect(r, color, rect.x, rect.y, rect.w, rect.h);
    
    // clamp radius
    if (radius > rect.w / 2) radius = rect.w / 2;
    if (radius > rect.h / 2) radius = rect.h / 2;

    nq_renderer_set_draw_color(r, color);

    // 4 straight lines
    nq_renderer_draw_line(r, color, rect.x + radius, rect.y, rect.x + rect.w - radius - 1, rect.y); // top
    nq_renderer_draw_line(r, color, rect.x + radius, rect.y + rect.h - 1, rect.x + rect.w - radius - 1, rect.y + rect.h - 1); // bottom
    nq_renderer_draw_line(r, color, rect.x, rect.y + radius, rect.x, rect.y + rect.h - radius - 1); // left
    nq_renderer_draw_line(r, color, rect.x + rect.w - 1, rect.y + radius, rect.x + rect.w - 1, rect.y + rect.h - radius - 1); // right

    // 4 arcs
    int cx, cy;
    int x = radius - 1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    while (x >= y) {
        // TL quadrant
        cx = rect.x + radius; cy = rect.y + radius;
        nq_renderer_fill_rect(r, color, cx - x, cy - y, 1, 1);
        nq_renderer_fill_rect(r, color, cx - y, cy - x, 1, 1);
        
        // TR quadrant
        cx = rect.x + rect.w - radius - 1; cy = rect.y + radius;
        nq_renderer_fill_rect(r, color, cx + x, cy - y, 1, 1);
        nq_renderer_fill_rect(r, color, cx + y, cy - x, 1, 1);

        // BL quadrant
        cx = rect.x + radius; cy = rect.y + rect.h - radius - 1;
        nq_renderer_fill_rect(r, color, cx - x, cy + y, 1, 1);
        nq_renderer_fill_rect(r, color, cx - y, cy + x, 1, 1);

        // BR quadrant
        cx = rect.x + rect.w - radius - 1; cy = rect.y + rect.h - radius - 1;
        nq_renderer_fill_rect(r, color, cx + x, cy + y, 1, 1);
        nq_renderer_fill_rect(r, color, cx + y, cy + x, 1, 1);

        if (err <= 0) {
            y++;
            err += dy;
            dy += 2;
        }
        if (err > 0) {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }

    return 0;
}

int nq_renderer_draw_cross(NqRenderer *r, NqColor color, int x, int y, int size) {
    if (!r || size <= 0) return -1;
    
    int ret1 = nq_renderer_draw_line(r, color, x - size, y, x + size, y);
    int ret2 = nq_renderer_draw_line(r, color, x, y - size, x, y + size);
    
    return (ret1 == 0 && ret2 == 0) ? 0 : -1;
}
