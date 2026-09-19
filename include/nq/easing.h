/*
 * nq — easing functions library.
 *
 * All functions map a normalised time t ∈ [0, 1] to a progress value.
 * Values outside [0, 1] are not clamped; callers are responsible for
 * clamping when needed.
 *
 * Naming convention: nq_ease_<family>_<direction>
 *   Families : linear, quad, cubic, quart, quint, sine, expo, circ,
 *              back, bounce, elastic
 *   Directions: in, out, in_out
 *
 * All functions are static inline — no .c file required.
 *
 * Reference: https://easings.net/
 */
#ifndef NQ_EASING_H
#define NQ_EASING_H

#include <math.h>

/* ── helpers ─────────────────────────────────────────────────────────── */

#ifndef NQ_PI
#define NQ_PI 3.14159265358979323846f
#endif

/* ── linear ──────────────────────────────────────────────────────────── */

static inline float nq_ease_linear(float t) {
    return t;
}

/* ── quadratic ───────────────────────────────────────────────────────── */

static inline float nq_ease_quad_in(float t) {
    return t * t;
}

static inline float nq_ease_quad_out(float t) {
    return t * (2.0f - t);
}

static inline float nq_ease_quad_in_out(float t) {
    if (t < 0.5f) return 2.0f * t * t;
    return -1.0f + (4.0f - 2.0f * t) * t;
}

/* ── cubic ───────────────────────────────────────────────────────────── */

static inline float nq_ease_cubic_in(float t) {
    return t * t * t;
}

static inline float nq_ease_cubic_out(float t) {
    float s = t - 1.0f;
    return s * s * s + 1.0f;
}

static inline float nq_ease_cubic_in_out(float t) {
    if (t < 0.5f) return 4.0f * t * t * t;
    float s = 2.0f * t - 2.0f;
    return 0.5f * s * s * s + 1.0f;
}

/* ── quartic ─────────────────────────────────────────────────────────── */

static inline float nq_ease_quart_in(float t) {
    return t * t * t * t;
}

static inline float nq_ease_quart_out(float t) {
    float s = t - 1.0f;
    return 1.0f - s * s * s * s;
}

static inline float nq_ease_quart_in_out(float t) {
    if (t < 0.5f) return 8.0f * t * t * t * t;
    float s = t - 1.0f;
    return 1.0f - 8.0f * s * s * s * s;
}

/* ── quintic ─────────────────────────────────────────────────────────── */

static inline float nq_ease_quint_in(float t) {
    return t * t * t * t * t;
}

static inline float nq_ease_quint_out(float t) {
    float s = t - 1.0f;
    return s * s * s * s * s + 1.0f;
}

static inline float nq_ease_quint_in_out(float t) {
    if (t < 0.5f) return 16.0f * t * t * t * t * t;
    float s = 2.0f * t - 2.0f;
    return 0.5f * s * s * s * s * s + 1.0f;
}

/* ── sine ────────────────────────────────────────────────────────────── */

static inline float nq_ease_sine_in(float t) {
    return 1.0f - cosf(t * NQ_PI * 0.5f);
}

static inline float nq_ease_sine_out(float t) {
    return sinf(t * NQ_PI * 0.5f);
}

static inline float nq_ease_sine_in_out(float t) {
    return 0.5f * (1.0f - cosf(t * NQ_PI));
}

/* ── exponential ─────────────────────────────────────────────────────── */

static inline float nq_ease_expo_in(float t) {
    return (t == 0.0f) ? 0.0f : powf(2.0f, 10.0f * t - 10.0f);
}

static inline float nq_ease_expo_out(float t) {
    return (t == 1.0f) ? 1.0f : 1.0f - powf(2.0f, -10.0f * t);
}

static inline float nq_ease_expo_in_out(float t) {
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    if (t < 0.5f) return 0.5f * powf(2.0f, 20.0f * t - 10.0f);
    return 0.5f * (2.0f - powf(2.0f, -20.0f * t + 10.0f));
}

/* ── circular ────────────────────────────────────────────────────────── */

static inline float nq_ease_circ_in(float t) {
    return 1.0f - sqrtf(1.0f - t * t);
}

static inline float nq_ease_circ_out(float t) {
    float s = t - 1.0f;
    return sqrtf(1.0f - s * s);
}

static inline float nq_ease_circ_in_out(float t) {
    if (t < 0.5f) return 0.5f * (1.0f - sqrtf(1.0f - 4.0f * t * t));
    float s = 2.0f * t - 2.0f;
    return 0.5f * (sqrtf(1.0f - s * s) + 1.0f);
}

/* ── back (overshoot) ────────────────────────────────────────────────── */

/* Default overshoot constant s = 1.70158 */
#define NQ_EASE_BACK_S  1.70158f
#define NQ_EASE_BACK_S2 2.5949095f   /* s * 1.525 */

static inline float nq_ease_back_in(float t) {
    return t * t * ((NQ_EASE_BACK_S + 1.0f) * t - NQ_EASE_BACK_S);
}

static inline float nq_ease_back_out(float t) {
    float s = t - 1.0f;
    return s * s * ((NQ_EASE_BACK_S + 1.0f) * s + NQ_EASE_BACK_S) + 1.0f;
}

static inline float nq_ease_back_in_out(float t) {
    if (t < 0.5f) {
        return 0.5f * (4.0f * t * t * ((NQ_EASE_BACK_S2 + 1.0f) * 2.0f * t - NQ_EASE_BACK_S2));
    }
    float s = 2.0f * t - 2.0f;
    return 0.5f * (s * s * ((NQ_EASE_BACK_S2 + 1.0f) * s + NQ_EASE_BACK_S2) + 2.0f);
}

/* ── bounce ──────────────────────────────────────────────────────────── */

static inline float nq_ease_bounce_out(float t);   /* forward declaration */

static inline float nq_ease_bounce_in(float t) {
    return 1.0f - nq_ease_bounce_out(1.0f - t);
}

static inline float nq_ease_bounce_out(float t) {
    if (t < 1.0f / 2.75f) {
        return 7.5625f * t * t;
    } else if (t < 2.0f / 2.75f) {
        t -= 1.5f / 2.75f;
        return 7.5625f * t * t + 0.75f;
    } else if (t < 2.5f / 2.75f) {
        t -= 2.25f / 2.75f;
        return 7.5625f * t * t + 0.9375f;
    } else {
        t -= 2.625f / 2.75f;
        return 7.5625f * t * t + 0.984375f;
    }
}

static inline float nq_ease_bounce_in_out(float t) {
    if (t < 0.5f) return 0.5f * nq_ease_bounce_in(t * 2.0f);
    return 0.5f * nq_ease_bounce_out(t * 2.0f - 1.0f) + 0.5f;
}

/* ── elastic ─────────────────────────────────────────────────────────── */

/* Amplitude = 1, period = 0.3 (standard easings.net defaults). */
#define NQ_EASE_ELASTIC_P    0.3f
#define NQ_EASE_ELASTIC_P_IO 0.45f   /* period for in_out = 0.3 * 1.5 */

static inline float nq_ease_elastic_in(float t) {
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    return -powf(2.0f, 10.0f * t - 10.0f) *
           sinf((t * 10.0f - 10.75f) * (2.0f * NQ_PI / NQ_EASE_ELASTIC_P));
}

static inline float nq_ease_elastic_out(float t) {
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    return powf(2.0f, -10.0f * t) *
           sinf((t * 10.0f - 0.75f) * (2.0f * NQ_PI / NQ_EASE_ELASTIC_P)) + 1.0f;
}

static inline float nq_ease_elastic_in_out(float t) {
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    if (t < 0.5f) {
        return -0.5f * powf(2.0f, 20.0f * t - 10.0f) *
               sinf((20.0f * t - 11.125f) * (2.0f * NQ_PI / NQ_EASE_ELASTIC_P_IO));
    }
    return 0.5f * powf(2.0f, -20.0f * t + 10.0f) *
           sinf((20.0f * t - 11.125f) * (2.0f * NQ_PI / NQ_EASE_ELASTIC_P_IO)) + 1.0f;
}

#endif /* NQ_EASING_H */
