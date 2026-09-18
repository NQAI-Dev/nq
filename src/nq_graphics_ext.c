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

int nq_renderer_draw_thick_line(NqRenderer *r, NqColor color, int x1, int y1, int x2, int y2, int thickness) {
    if (!r || thickness <= 0) return -1;
    if (thickness == 1) {
        return nq_renderer_draw_line(r, color, x1, y1, x2, y2);
    }
    
    // We would ideally compute the polygon points and draw it.
    // Given SDL3's limitations with filled polygons out of the box (in standard 2D render API),
    // drawing a thick line with pure SDL_RenderLines without a dedicated geometric rasterizer
    // is tricky. Since NqRenderer uses the generic SDL renderer, we'll approximate with multiple lines
    // along the perpendicular vector for simplicity.
    // For a real game, passing a 1x1 white texture to RenderGeometry is correct, but we lack
    // direct access to texture/geometry API here without polluting the dependency scope.
    
    int dx = x2 - x1;
    int dy = y2 - y1;
    
    // Rough length squared
    float len_sq = (float)(dx * dx + dy * dy);
    if (len_sq < 1.0f) return 0;
    
    // Perpendicular vector normalized
    #include <math.h>
    float len = sqrtf(len_sq);
    float nx = -dy / len;
    float ny =  dx / len;
    
    int ret = 0;
    int half_thickness = thickness / 2;
    for (int i = -half_thickness; i <= half_thickness; i++) {
        int ox = (int)(nx * i);
        int oy = (int)(ny * i);
        if (nq_renderer_draw_line(r, color, x1 + ox, y1 + oy, x2 + ox, y2 + oy) != 0) {
            ret = -1;
        }
    }
    return ret;
}

int nq_renderer_draw_point(NqRenderer *r, NqColor color, int x, int y) {
    if (!r) return -1;
    // We can draw a 1px line to itself, which SDL standardizes as a point
    return nq_renderer_draw_line(r, color, x, y, x, y);
}

int nq_renderer_draw_grid(NqRenderer *r, NqColor color, int x, int y, int w, int h, int cell_w, int cell_h) {
    if (!r || cell_w <= 0 || cell_h <= 0) return -1;
    if (w <= 0 || h <= 0) return 0;
    
    int ret = 0;
    // Draw vertical lines
    for (int i = 0; i <= w; i += cell_w) {
        if (nq_renderer_draw_line(r, color, x + i, y, x + i, y + h) != 0) {
            ret = -1;
        }
    }
    // Ensure the rightmost edge is drawn if it doesn't align exactly with cell_w
    if (w % cell_w != 0) {
        if (nq_renderer_draw_line(r, color, x + w, y, x + w, y + h) != 0) ret = -1;
    }
    
    // Draw horizontal lines
    for (int j = 0; j <= h; j += cell_h) {
        if (nq_renderer_draw_line(r, color, x, y + j, x + w, y + j) != 0) {
            ret = -1;
        }
    }
    // Ensure the bottom edge is drawn if it doesn't align exactly with cell_h
    if (h % cell_h != 0) {
        if (nq_renderer_draw_line(r, color, x, y + h, x + w, y + h) != 0) ret = -1;
    }
    
    return ret;
}
