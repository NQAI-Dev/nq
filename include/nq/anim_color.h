/*
 * nq — color animator.
 *
 * Per-frame color animator. Mirrors NqAnimFloat exactly but the value
 * type is NqColor (4 channels RGBA). Used by nq_action_tween via the
 * color-typed tween variant (separate tick), and standalone for any
 * "lerp between two colors over duration" need.
 *
 * Lifetime: same as NqAnimFloat — caller-owned struct (stack or heap).
 * 8 bytes of value state, ~24 bytes total.
 */
#ifndef NQ_ANIM_COLOR_H
#define NQ_ANIM_COLOR_H

#include <nq/action.h>
#include <nq/animation.h>
#include <nq/color.h>
#include <nq/graphics.h>  /* NqColor */
#include <nq/tween.h>

typedef struct {
    NqColor  from;
    NqColor  to;
    float    duration_seconds;
    float    elapsed_seconds;
    int      active;
    int      reverse;       /* to < from — lerp goes "down" */
    NqEaseKind ease;
} NqAnimColor;

void nq_anim_color_init(NqAnimColor *a,
                       NqColor from, NqColor to,
                       float duration_seconds, NqEaseKind ease);

int   nq_anim_color_update(NqAnimColor *a, float dt_seconds);
int   nq_anim_color_done(const NqAnimColor *a);
NqColor nq_anim_color_value(const NqAnimColor *a);
void  nq_anim_color_restart(NqAnimColor *a);

#endif /* NQ_ANIM_COLOR_H */
