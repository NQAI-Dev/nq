/*
 * nq — animation primitives.
 *
 * Per-value animators with elapsed/duration/easing — composable building
 * blocks for higher-level animation systems (timeline tracks, tween
 * managers, scene-graph property animators). Track-based NqAnim follows
 * in a follow-up tick; this header ships the per-value primitive only.
 *
 * Lifetime: animators can be stack-allocated (their state is plain old
 * data) or heap-allocated (use nq_anim_* helpers). Stack form avoids
 * malloc during gameplay; heap form is convenient for short-lived ones.
 */
#ifndef NQ_ANIMATION_H
#define NQ_ANIMATION_H

#include <nq/tween.h>

typedef struct {
    NqEaseKind ease;
    float      duration_seconds;
    float      elapsed_seconds;
    float      from;
    float      to;
    int        active;       /* 1 while running, 0 when done */
    int        reverse;      /* if non-zero, from and to swapped (from > to) */
} NqAnimFloat;

/* Initialize in place. duration_seconds must be > 0; values <= 0 are
 * clamped to 1ms to avoid divide-by-zero in nq_anim_float_value. */
void nq_anim_float_init(NqAnimFloat *a,
                        float from, float to,
                        float duration_seconds,
                        NqEaseKind ease);

/* Advance the animation by dt seconds. dt < 0 is treated as 0.
 * Returns 1 if the animation just completed on this call, 0 otherwise. */
int  nq_anim_float_update(NqAnimFloat *a, float dt_seconds);

/* 1 once elapsed >= duration, 0 while running. */
int  nq_anim_float_done(const NqAnimFloat *a);

/* Current eased value at time = elapsed. If done, returns `to`. */
float nq_anim_float_value(const NqAnimFloat *a);

/* Reset an existing animator to start over with the same params. */
void nq_anim_float_restart(NqAnimFloat *a);

#endif /* NQ_ANIMATION_H */
