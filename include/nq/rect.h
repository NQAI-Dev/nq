/*
 * nq — integer axis-aligned rectangle.
 *
 * Pure C, no SDL3 dependency. The collision and layout layers compose on
 * top. Half-open on max edges by convention: a rect {x, y, w, h} covers
 * x in [x, x+w) and y in [y, y+h). Collisions, contains() and intersects()
 * use the same convention so two rects sharing an edge report as touching
 * only on the inclusive side.
 */
#ifndef NQ_RECT_H
#define NQ_RECT_H

#include <stdbool.h>

#include <nq/vec.h>

typedef struct {
    int x;
    int y;
    int w;
    int h;
} NqRect;

static inline NqRect nq_rect(int x, int y, int w, int h) {
    NqRect r = { x, y, w, h };
    return r;
}

static inline bool nq_rect_empty(NqRect r) {
    return r.w <= 0 || r.h <= 0;
}

static inline bool nq_rect_eq(NqRect a, NqRect b) {
    return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
}

/* Half-open: a point with x == r.x + r.w is OUTSIDE. */
static inline bool nq_rect_contains(NqRect r, NqVec2i p) {
    return p.x >= r.x && p.x < r.x + r.w &&
           p.y >= r.y && p.y < r.y + r.h;
}

/* Standard AABB overlap test (half-open on max edges). */
static inline bool nq_rect_intersects(NqRect a, NqRect b) {
    if (nq_rect_empty(a) || nq_rect_empty(b)) {
        return false;
    }
    return a.x < b.x + b.w && a.x + a.w > b.x &&
           a.y < b.y + b.h && a.y + a.h > b.y;
}

static inline NqRect nq_rect_intersection(NqRect a, NqRect b) {
    int x1 = a.x > b.x ? a.x : b.x;
    int y1 = a.y > b.y ? a.y : b.y;
    int x2 = (a.x + a.w) < (b.x + b.w) ? (a.x + a.w) : (b.x + b.w);
    int y2 = (a.y + a.h) < (b.y + b.h) ? (a.y + a.h) : (b.y + b.h);
    if (x2 <= x1 || y2 <= y1) {
        return nq_rect(0, 0, 0, 0);
    }
    return nq_rect(x1, y1, x2 - x1, y2 - y1);
}

/* Smallest rect enclosing both. Always non-empty when either input is
 * non-empty, even if they don't overlap. */
static inline NqRect nq_rect_union(NqRect a, NqRect b) {
    if (nq_rect_empty(a)) return b;
    if (nq_rect_empty(b)) return a;
    int x1 = a.x < b.x ? a.x : b.x;
    int y1 = a.y < b.y ? a.y : b.y;
    int x2 = (a.x + a.w) > (b.x + b.w) ? (a.x + a.w) : (b.x + b.w);
    int y2 = (a.y + a.h) > (b.y + b.h) ? (a.y + a.h) : (b.y + b.h);
    return nq_rect(x1, y1, x2 - x1, y2 - y1);
}

static inline NqRect nq_rect_inflate(NqRect r, int dx, int dy) {
    return nq_rect(r.x - dx, r.y - dy, r.w + 2 * dx, r.h + 2 * dy);
}

/* Linear interpolation between two rects. Each component (x, y, w, h) is
 * interpolated independently and rounded to int. t is clamped to [0,1]
 * so this composes cleanly with `nq_ease` outputs. */
static inline NqRect nq_rect_lerp(NqRect a, NqRect b, float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return nq_rect(
        (int)(a.x + (b.x - a.x) * t),
        (int)(a.y + (b.y - a.y) * t),
        (int)(a.w + (b.w - a.w) * t),
        (int)(a.h + (b.h - a.h) * t)
    );
}

#endif /* NQ_RECT_H */
