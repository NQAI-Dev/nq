/*
 * nq — rect animator.
 *
 * Per-frame rect animator. Mirrors NqAnimFloat / NqAnimColor exactly
 * but the value type is NqRect (x, y, w, h). The update path uses
 * `nq_rect_lerp`, which clamps t to [0,1] so this composes cleanly with
 * `nq_ease` outputs.
 *
 * Caller pattern is identical to the other animators:
 *   NqAnimRect anim;
 *   nq_anim_rect_init(&anim, from, to, duration_seconds, NQ_EASE_QUAD_OUT);
 *   // each frame:
 *   nq_anim_rect_update(&anim, dt);
 *   if (!nq_anim_rect_done(&anim)) { ... use nq_anim_rect_value(&anim) ... }
 *   else { nq_anim_rect_restart(&anim); }
 *
 * Or wrap in `nq_action_tween_rect` for action-framework scheduling.
 */
#ifndef NQ_ANIM_RECT_H
#define NQ_ANIM_RECT_H

#include <nq/animation.h>
#include <nq/graphics.h>  /* NqRect */
#include <nq/rect.h>
#include <nq/tween.h>

typedef struct {
    NqRect   from;
    NqRect   to;
    float    duration_seconds;
    float    elapsed_seconds;
    int      active;
    int      reverse;       /* set if to < from on any axis — lerp goes "down" */
    NqEaseKind ease;
} NqAnimRect;

void nq_anim_rect_init(NqAnimRect *a,
                      NqRect from, NqRect to,
                      float duration_seconds, NqEaseKind ease);

int     nq_anim_rect_update(NqAnimRect *a, float dt_seconds);
int     nq_anim_rect_done(const NqAnimRect *a);
NqRect  nq_anim_rect_value(const NqAnimRect *a);
void    nq_anim_rect_restart(NqAnimRect *a);

#endif /* NQ_ANIM_RECT_H */
