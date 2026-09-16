/*
 * nq — vec2 animator.
 *
 * Per-frame float-vector animator. Mirrors NqAnimFloat / NqAnimColor /
 * NqAnimRect: drives an NqVec2f from `from` to `to` over
 * `duration_seconds`, with `nq_vec2f_lerp` + `nq_ease_lerp` composing
 * the value curve.
 *
 * Use case: sprite position tweens, camera shake easing, mouse-smooth
 * follow. For axis-typed coordinates use nq_anim_color / nq_anim_rect.
 */
#ifndef NQ_ANIM_VEC2_H
#define NQ_ANIM_VEC2_H

#include <nq/animation.h>
#include <nq/graphics.h>  /* NqVec2f */
#include <nq/tween.h>
#include <nq/vec.h>

typedef struct {
    NqVec2f    from;
    NqVec2f    to;
    float      duration_seconds;
    float      elapsed_seconds;
    int        active;
    int        reverse;       /* set if to < from on any axis — lerp goes "down" */
    NqEaseKind  ease;
} NqAnimVec2;

void nq_anim_vec2_init(NqAnimVec2 *a,
                       NqVec2f from, NqVec2f to,
                       float duration_seconds, NqEaseKind ease);

int     nq_anim_vec2_update(NqAnimVec2 *a, float dt_seconds);
int     nq_anim_vec2_done(const NqAnimVec2 *a);
NqVec2f nq_anim_vec2_value(const NqAnimVec2 *a);
void    nq_anim_vec2_restart(NqAnimVec2 *a);

#endif /* NQ_ANIM_VEC2_H */
