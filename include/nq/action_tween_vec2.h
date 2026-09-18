/*
 * nq — vec2 tween action.
 *
 * NqAction wrapper that drives a single NqAnimVec2 from the action tick.
 * Composes nq_clock (dt source) + nq_tween (ease curve) + nq_anim_vec2
 * (vec2 lerp primitive) into the action framework, so any
 * nq_action_tween_vec2 instance can be scheduled from an action manager
 * alongside delays, sequences, etc.
 *
 * Lifetime: same rules as nq_action. Caller owns the struct. Destroy with
 * nq_action_tween_vec2_destroy (which also calls nq_action_destroy under the
 * hood — never call both).
 */
#ifndef NQ_ACTION_TWEEN_VEC2_H
#define NQ_ACTION_TWEEN_VEC2_H

#include <nq/action.h>
#include <nq/anim_vec2.h>

typedef struct {
    NqAction *action;
    NqAnimVec2 *anim;
    nq_action_tick_fn custom_tick;
    void *custom_user;
} NqActionTweenVec2;

NqActionTweenVec2 *nq_action_tween_vec2_create(nq_action_tick_fn tick, void *user);
void nq_action_tween_vec2_set_anim(NqActionTweenVec2 *t, NqAnimVec2 *anim);
void nq_action_tween_vec2_destroy(NqActionTweenVec2 *t);
NqActionState nq_action_tween_vec2_update(NqActionTweenVec2 *t, float dt);
NqAction *nq_action_tween_vec2_action(NqActionTweenVec2 *t);

#endif /* NQ_ACTION_TWEEN_VEC2_H */
