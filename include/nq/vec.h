/*
 * nq — 2D vector primitives.
 *
 * Pure C, no SDL3 dependency. Lives in the core math layer; everything
 * above (rect, node, camera, physics) composes on top.
 */
#ifndef NQ_VEC_H
#define NQ_VEC_H

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

#endif /* NQ_VEC_H */
