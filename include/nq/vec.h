/*
 * nq — 2D vector primitives.
 *
 * Pure C, no SDL3 dependency. Lives in the core math layer; everything
 * above (rect, node, camera, physics) composes on top.
 */
#ifndef NQ_VEC_H
#define NQ_VEC_H

#include <math.h>
#include <stdbool.h>

typedef struct {
    int x;
    int y;
} NqVec2i;

typedef struct {
    float x;
    float y;
} NqVec2f;

static inline NqVec2i nq_vec2i(int x, int y) {
    NqVec2i v = { x, y };
    return v;
}

static inline NqVec2f nq_vec2f(float x, float y) {
    NqVec2f v = { x, y };
    return v;
}

static inline NqVec2i nq_vec2i_add(NqVec2i a, NqVec2i b) {
    return nq_vec2i(a.x + b.x, a.y + b.y);
}

static inline NqVec2i nq_vec2i_sub(NqVec2i a, NqVec2i b) {
    return nq_vec2i(a.x - b.x, a.y - b.y);
}

static inline NqVec2i nq_vec2i_scale(NqVec2i v, int s) {
    return nq_vec2i(v.x * s, v.y * s);
}

static inline bool nq_vec2i_eq(NqVec2i a, NqVec2i b) {
    return a.x == b.x && a.y == b.y;
}

static inline NqVec2f nq_vec2f_add(NqVec2f a, NqVec2f b) {
    return nq_vec2f(a.x + b.x, a.y + b.y);
}

static inline NqVec2f nq_vec2f_sub(NqVec2f a, NqVec2f b) {
    return nq_vec2f(a.x - b.x, a.y - b.y);
}

static inline NqVec2f nq_vec2f_scale(NqVec2f v, float s) {
    return nq_vec2f(v.x * s, v.y * s);
}

/* Linear interpolation between two vectors. Each component is
 * interpolated independently. t is clamped to [0,1] so these compose
 * cleanly with `nq_ease` outputs. */
static inline NqVec2i nq_vec2i_lerp(NqVec2i a, NqVec2i b, float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return nq_vec2i(
        (int)(a.x + (b.x - a.x) * t),
        (int)(a.y + (b.y - a.y) * t)
    );
}

static inline NqVec2f nq_vec2f_lerp(NqVec2f a, NqVec2f b, float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return nq_vec2f(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t
    );
}

static inline bool nq_vec2f_eq(NqVec2f a, NqVec2f b) {
    return a.x == b.x && a.y == b.y;
}

/* Dot product: a.x*b.x + a.y*b.y.
 * Positive: vectors point in roughly the same direction.
 * Zero: perpendicular. Negative: opposing. */
static inline float nq_vec2f_dot(NqVec2f a, NqVec2f b) {
    return a.x * b.x + a.y * b.y;
}

/* 2D cross product (scalar z-component of 3D cross).
 * nq_vec2f_cross(a, b) > 0  → b is counter-clockwise from a.
 * nq_vec2f_cross(a, b) < 0  → b is clockwise from a.
 * nq_vec2f_cross(a, b) == 0 → collinear. */
static inline float nq_vec2f_cross(NqVec2f a, NqVec2f b) {
    return a.x * b.y - a.y * b.x;
}

/* Squared length — avoids sqrtf; useful for magnitude comparisons. */
static inline float nq_vec2f_length_sq(NqVec2f v) {
    return v.x * v.x + v.y * v.y;
}

/* Euclidean length. Returns 0 for the zero vector. */
static inline float nq_vec2f_length(NqVec2f v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

/* Unit vector in the direction of v.
 * Returns (0, 0) for the zero vector (safe; no division by zero). */
static inline NqVec2f nq_vec2f_normalize(NqVec2f v) {
    float len = nq_vec2f_length(v);
    if (len == 0.0f) return nq_vec2f(0.0f, 0.0f);
    return nq_vec2f(v.x / len, v.y / len);
}

/* Counter-clockwise perpendicular: rotates v by 90° CCW → (-y, x). */
static inline NqVec2f nq_vec2f_perp(NqVec2f v) {
    return nq_vec2f(-v.y, v.x);
}

/* Reflect v across a surface with (normalised) normal n.
 * v_r = v - 2*(v·n)*n  — standard specular-reflection formula.
 * n must be a unit vector; result is undefined for non-unit n. */
static inline NqVec2f nq_vec2f_reflect(NqVec2f v, NqVec2f n) {
    float d = 2.0f * nq_vec2f_dot(v, n);
    return nq_vec2f(v.x - d * n.x, v.y - d * n.y);
}

/* Integer dot / cross for NqVec2i. */
static inline int nq_vec2i_dot(NqVec2i a, NqVec2i b) {
    return a.x * b.x + a.y * b.y;
}

static inline int nq_vec2i_cross(NqVec2i a, NqVec2i b) {
    return a.x * b.y - a.y * b.x;
}

#endif /* NQ_VEC_H */
