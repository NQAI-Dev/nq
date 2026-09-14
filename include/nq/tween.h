/*
 * nq — easing functions and value interpolation.
 *
 * Pure functions, no state, no globals. t is in [0,1] (clamped at the
 * public API level); result f(t) is in [0,1]. For value interpolation
 * between two scalars, use nq_ease_lerp / nq_lerp — these take the
 * ease curve and lerp between (a, b) on the eased t.
 *
 * Standard easing function vocabulary; mathematical forms cited from
 * easings.net (Robert Penner's equations, public domain).
 */
#ifndef NQ_TWEEN_H
#define NQ_TWEEN_H

typedef enum {
    NQ_EASE_LINEAR = 0,
    NQ_EASE_QUAD_IN,
    NQ_EASE_QUAD_OUT,
    NQ_EASE_QUAD_IN_OUT,
    NQ_EASE_CUBIC_IN,
    NQ_EASE_CUBIC_OUT,
    NQ_EASE_CUBIC_IN_OUT,
    NQ_EASE_SINE_IN,
    NQ_EASE_SINE_OUT,
    NQ_EASE_SINE_IN_OUT,
    NQ_EASE_EXPO_IN,
    NQ_EASE_EXPO_OUT,
} NqEaseKind;

/* Apply the named easing curve to t in [0,1]. Out-of-range t is clamped.
 * Returns f(t) in [0,1] for all curves other than exponential early-frames
 * which dip slightly below 0 in the canonical formulation; the clamping
 * keeps the returned value in [0,1] for any input. */
float nq_ease(NqEaseKind kind, float t);
float nq_lerp(float a, float b, float t);
float nq_ease_lerp(NqEaseKind kind, float a, float b, float t);

/* Named-easing direct accessors, one per kind. Useful when the curve is
 * resolved at compile-time and a switch would be wasted work. */
float nq_ease_linear(float t);
float nq_ease_quad_in(float t);
float nq_ease_quad_out(float t);
float nq_ease_quad_in_out(float t);
float nq_ease_cubic_in(float t);
float nq_ease_cubic_out(float t);
float nq_ease_cubic_in_out(float t);
float nq_ease_sine_in(float t);
float nq_ease_sine_out(float t);
float nq_ease_sine_in_out(float t);
float nq_ease_expo_in(float t);
float nq_ease_expo_out(float t);

#endif /* NQ_TWEEN_H */
