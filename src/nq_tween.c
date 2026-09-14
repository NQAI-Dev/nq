#include "nq/tween.h"

#include <math.h>

/* Mathematical forms from easings.net (Robert Penner, public domain).
 * Pure functions; t in [0,1], return in [0,1]. */

float nq_ease_linear(float t) {
    return t;
}

float nq_ease_quad_in(float t) {
    return t * t;
}

float nq_ease_quad_out(float t) {
    return t * (2.0f - t);
}

float nq_ease_quad_in_out(float t) {
    return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
}

float nq_ease_cubic_in(float t) {
    return t * t * t;
}

float nq_ease_cubic_out(float t) {
    float u = 1.0f - t;
    return 1.0f - u * u * u;
}

float nq_ease_cubic_in_out(float t) {
    return t < 0.5f
        ? 4.0f * t * t * t
        : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

float nq_ease_sine_in(float t) {
    return 1.0f - cosf(t * 1.5707963f);   /* 1 - cos(t * pi/2) */
}

float nq_ease_sine_out(float t) {
    return sinf(t * 1.5707963f);            /* sin(t * pi/2) */
}

float nq_ease_sine_in_out(float t) {
    return -(cosf(3.1415927f * t) - 1.0f) / 2.0f;
}

float nq_ease_expo_in(float t) {
    /* Canonical formulation returns 0 at t=0 and t→∞ for t<0. We use a
     * clamped form: returns near-zero for tiny t (avoids producing a 0
     * render frame at the very start), approaches 1 quickly for t > 0. */
    return t == 0.0f ? 0.0f : powf(2.0f, 10.0f * t - 10.0f);
}

float nq_ease_expo_out(float t) {
    return t == 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f * t);
}

float nq_ease(NqEaseKind kind, float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    switch (kind) {
        case NQ_EASE_LINEAR:        return nq_ease_linear(t);
        case NQ_EASE_QUAD_IN:       return nq_ease_quad_in(t);
        case NQ_EASE_QUAD_OUT:      return nq_ease_quad_out(t);
        case NQ_EASE_QUAD_IN_OUT:   return nq_ease_quad_in_out(t);
        case NQ_EASE_CUBIC_IN:      return nq_ease_cubic_in(t);
        case NQ_EASE_CUBIC_OUT:     return nq_ease_cubic_out(t);
        case NQ_EASE_CUBIC_IN_OUT:  return nq_ease_cubic_in_out(t);
        case NQ_EASE_SINE_IN:       return nq_ease_sine_in(t);
        case NQ_EASE_SINE_OUT:      return nq_ease_sine_out(t);
        case NQ_EASE_SINE_IN_OUT:   return nq_ease_sine_in_out(t);
        case NQ_EASE_EXPO_IN:       return nq_ease_expo_in(t);
        case NQ_EASE_EXPO_OUT:      return nq_ease_expo_out(t);
    }
    return t;  /* unreachable; suppress compiler warning */
}

float nq_lerp(float a, float b, float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return a + (b - a) * t;
}

float nq_ease_lerp(NqEaseKind kind, float a, float b, float t) {
    return nq_lerp(a, b, nq_ease(kind, t));
}
