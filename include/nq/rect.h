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

#include <math.h>
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

/* Center point of the rectangle (as NqVec2i — top-left + size/2,
 * truncated toward zero). Empty rect (w<=0 or h<=0) returns the
 * top-left corner. */
static inline NqVec2i nq_rect_center(NqRect r) {
    return nq_vec2i(r.x + r.w / 2, r.y + r.h / 2);
}

/* Circle-vs-circle collision: returns 1 if two circles (radius r1
 * centred at (cx1, cy1), radius r2 centred at (cx2, cy2)) overlap or
 * touch, 0 otherwise. Pure integer math via squared-distance compare
 * — no sqrt, no float. Empty/negative radii behave like 0 (a point). */
static inline int nq_circle_overlap(int cx1, int cy1, int r1,
                                    int cx2, int cy2, int r2) {
    int dx = cx2 - cx1;
    int dy = cy2 - cy1;
    int r  = (r1 < 0 ? 0 : r1) + (r2 < 0 ? 0 : r2);
    /* Saturated subtract via long: avoid overflow on huge radii. */
    long dist_sq = (long)dx * dx + (long)dy * dy;
    long r_sum   = (long)r * r;
    return dist_sq <= r_sum;
}

/* Float variant of nq_circle_overlap. Useful for physics where
 * positions / radii are stored as float (e.g. post-multiplication by
 * a dt-derived scale). Same algorithm, no overflow handling. */
static inline int nq_circle_overlap_f(float cx1, float cy1, float r1,
                                      float cx2, float cy2, float r2) {
    if (r1 < 0.0f) r1 = 0.0f;
    if (r2 < 0.0f) r2 = 0.0f;
    float dx   = cx2 - cx1;
    float dy   = cy2 - cy1;
    float r    = r1 + r2;
    return (dx * dx + dy * dy) <= (r * r);
}

/* Circle-vs-rect collision: returns 1 if a circle of `radius` centred
 * at (cx, cy) overlaps (or touches) the rect, 0 otherwise. Useful for
 * entity-radius-vs-tile / particle-vs-wall checks. Edge cases:
 * - Empty rect (w<=0 || h<=0): no overlap → returns 0
 * - radius <= 0: only returns 1 if the centre point is inside the rect
 * - Negative radius: undefined input, treated as radius = 0 */
static inline int nq_rect_contains_circle(NqRect r, int cx, int cy, int radius) {
    if (radius <= 0) return nq_rect_contains(r, nq_vec2i(cx, cy));
    if (r.w <= 0 || r.h <= 0) return 0;
    /* Closest point on the rect to the circle centre. */
    int min_x = (cx < r.x) ? r.x : (cx > r.x + r.w ? r.x + r.w : cx);
    int min_y = (cy < r.y) ? r.y : (cy > r.y + r.h ? r.y + r.h : cy);
    int dx = cx - min_x;
    int dy = cy - min_y;
    return (dx * dx + dy * dy) <= (radius * radius);
}

/* Circle-vs-rect *intersection*: returns the rect of overlap between
 * a circle of `radius` centred at (cx, cy) and the rect `r`. Useful
 * for swept-volume collision (knowing WHERE two objects intersect,
 * not just whether they do). The returned rect is the box bounding
 * the circle-rect overlap region; for most cases (where the circle
 * is much smaller than the rect), this is a tight approximation.
 *
 * Edge cases:
 * - radius <= 0: returns `nq_rect_intersection(r, nq_rect(cx, cy, 0, 0))`
 * - No overlap (circle entirely outside rect): returns an empty rect
 *   (w<=0 || h<=0)
 * - Empty rect (r.w<=0 || r.h<=0): returns an empty rect
 */
/* Circle-vs-circle penetration resolution. Returns the vector to
 * translate circle B (centred at bx, by with radius br) so it just
 * touches circle A (centred at ax, ay with radius ar) — i.e. pushes
 * B AWAY from A along the contact axis by the overlap distance.
 *
 * Returns (0, 0) if the circles are not currently overlapping. If
 * the circles are concentric (distance 0), pushes B along +X by
 * (ar + br) — an arbitrary axis but well-defined behaviour.
 *
 * Most useful form for physics: each frame, after checking overlap
 * (nq_circle_overlap), if non-zero, call this and translate B by
 * the returned vector. This makes a stable, non-sticking collision
 * response without needing a full physics engine.
 */
static inline NqVec2f nq_circle_penetration_vector_f(float ax, float ay, float ar,
                                                   float bx, float by, float br) {
    float dx = bx - ax;
    float dy = by - ay;
    float dist_sq = dx * dx + dy * dy;
    if (dist_sq == 0.0f) {
        /* Concentric: push B in +X by total radii. */
        return nq_vec2f(ar + br, 0.0f);
    }
    float dist = sqrtf(dist_sq);
    float overlap = (ar + br) - dist;
    if (overlap <= 0.0f) {
        return nq_vec2f(0.0f, 0.0f);
    }
    return nq_vec2f(dx / dist * overlap, dy / dist * overlap);
}

/* Integer variant — same algorithm, integer arithmetic via Pythagorean
 * approximation not needed (we still call sqrtf to get the unit
 * direction, but with integer radii/distances). */
static inline NqVec2f nq_circle_penetration_vector(int ax, int ay, int ar,
                                                  int bx, int by, int br) {
    return nq_circle_penetration_vector_f(
        (float)ax, (float)ay, (float)ar,
        (float)bx, (float)by, (float)br
    );
}

/* Rect-as-obstacle vs mobile-circle penetration resolution. Returns the
 * vector to translate the circle (centred at cx, cy with radius) so it
 * just touches the rect on the closest face / corner.
 *
 * Algorithm: find the closest point on the rect's surface to the
 * circle centre; if the centre is inside the rect, the closest point
 * is the centre itself (the circle is fully enveloped, push along
 * the shortest axis to the nearest edge). If the centre is outside,
 * the closest point is on a face or a corner — push along the
 * contact axis from that closest point outward by (radius - distance).
 *
 * Useful for: entity-with-radius vs tile-AABB collision response
 * (the most common 2D game collision shape pair).
 */
static inline NqVec2f nq_rect_penetration_vector_f(NqRect r, float cx, float cy, float radius) {
    /* Closest point on the rect's surface to (cx, cy). */
    float min_x = (cx < (float)r.x) ? (float)r.x : (cx > (float)(r.x + r.w) ? (float)(r.x + r.w) : cx);
    float min_y = (cy < (float)r.y) ? (float)r.y : (cy > (float)(r.y + r.h) ? (float)(r.y + r.h) : cy);
    float dx = cx - min_x;
    float dy = cy - min_y;
    float dist_sq = dx * dx + dy * dy;
    if (dist_sq >= radius * radius) {
        return nq_vec2f(0.0f, 0.0f);  /* no overlap */
    }
    if (dist_sq == 0.0f) {
        /* Centre is inside the rect — push along the shorter axis to
         * the nearest edge. */
        float left   = cx - (float)r.x;
        float right  = (float)(r.x + r.w) - cx;
        float top    = cy - (float)r.y;
        float bottom = (float)(r.y + r.h) - cy;
        float min_axis = left; int dir_x = -1; int dir_y = 0;
        if (right  < min_axis) { min_axis = right;  dir_x =  1; dir_y = 0; }
        if (top    < min_axis) { min_axis = top;    dir_x =  0; dir_y = -1; }
        if (bottom < min_axis) {                       dir_x =  0; dir_y =  1; }
        return nq_vec2f((float)dir_x * (min_axis + radius), (float)dir_y * (min_axis + radius));
    }
    float dist = sqrtf(dist_sq);
    float overlap = radius - dist;
    return nq_vec2f(-dx / dist * overlap, -dy / dist * overlap);
}

static inline NqRect nq_rect_intersect_circle(NqRect r, int cx, int cy, int radius) {
    /* Same circle-vs-rect no-overlap check as nq_rect_contains_circle's
     * counterpart — bail early when there is nothing to clip. */
    if (!nq_rect_contains_circle(r, cx, cy, radius)) {
        return nq_rect(0, 0, 0, 0);
    }
    /* The overlap box is the intersection of:
     *   A = (cx - radius, cy - radius, 2*radius, 2*radius)  — circle's AABB
     *   B = r                                             — original rect
     * Use nq_rect_intersection to compute the overlap. */
    NqRect circle_aabb = nq_rect(cx - radius, cy - radius, 2 * radius, 2 * radius);
    return nq_rect_intersection(r, circle_aabb);
}

/* Float variant of nq_rect_intersect_circle. Same algorithm, uses
 * nq_rect_contains_circle_f for the no-overlap early bail and
 * (cx - radius, cy - radius, 2*radius, 2*radius) rounded to int
 * for the circle AABB. Since rect components are ints, the final
 * intersection is also int-valued — float precision is preserved
 * only in the contained-test decision. */
/* Float variant of nq_rect_contains_circle. Useful for float-based
 * physics (post-multiply by dt scales, sub-pixel positioning, etc.).
 * Same algorithm: clamp circle centre to rect to find the closest
 * point on the rect's surface, then compare squared distance vs
 * radius². The rect is integer (x, y, w, h) — float versions of
 * the rect work the same way since float arithmetic is exact for
 * the integer-component clamp step. */
static inline int nq_rect_contains_circle_f(NqRect r, float cx, float cy, float radius) {
    if (radius <= 0.0f) {
        return nq_rect_contains(r, nq_vec2i((int)cx, (int)cy));
    }
    if (r.w <= 0 || r.h <= 0) return 0;
    /* Closest point on the rect to the circle centre. */
    float min_x = (cx < (float)r.x) ? (float)r.x : (cx > (float)(r.x + r.w) ? (float)(r.x + r.w) : cx);
    float min_y = (cy < (float)r.y) ? (float)r.y : (cy > (float)(r.y + r.h) ? (float)(r.y + r.h) : cy);
    float dx = cx - min_x;
    float dy = cy - min_y;
    return (dx * dx + dy * dy) <= (radius * radius);
}

static inline NqRect nq_rect_intersect_circle_f(NqRect r, float cx, float cy, float radius) {
    if (!nq_rect_contains_circle_f(r, cx, cy, radius)) {
        return nq_rect(0, 0, 0, 0);
    }
    int cxi = (int)cx;
    int cyi = (int)cy;
    int ri  = (int)radius;
    NqRect circle_aabb = nq_rect(cxi - ri, cyi - ri, 2 * ri, 2 * ri);
    return nq_rect_intersection(r, circle_aabb);
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
