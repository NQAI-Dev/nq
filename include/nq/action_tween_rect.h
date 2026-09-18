/*
 * nq — rect tween action.
 *
 * NqAction wrapper that drives a single NqAnimRect from the action tick.
 * Composes nq_clock (dt source) + nq_tween (ease curve) + nq_anim_rect
 * (rect lerp primitive) into the action framework, so any
 * nq_action_tween_rect instance can be scheduled from an action manager
 * alongside delays, sequences, etc.
 *
 * Lifetime: same rules as nq_action. Caller owns the struct. Destroy with
 * nq_action_tween_rect_destroy (which also calls nq_action_destroy under the
 * hood — never call both).
 */
#ifndef NQ_ACTION_TWEEN_RECT_H
#define NQ_ACTION_TWEEN_RECT_H

#include <nq/action.h>
#include <nq/anim_rect.h>

typedef struct {
    NqAction *action;
    NqAnimRect *anim;
    nq_action_tick_fn custom_tick;
    void *custom_user;
} NqActionTweenRect;

NqActionTweenRect *nq_action_tween_rect_create(nq_action_tick_fn tick, void *user);
void nq_action_tween_rect_set_anim(NqActionTweenRect *t, NqAnimRect *anim);
void nq_action_tween_rect_destroy(NqActionTweenRect *t);
NqActionState nq_action_tween_rect_update(NqActionTweenRect *t, float dt);
NqAction *nq_action_tween_rect_action(NqActionTweenRect *t);

#endif /* NQ_ACTION_TWEEN_RECT_H */
