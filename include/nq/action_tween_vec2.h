/*
 * nq — vec2 tween action.
 *
 * NqAction wrapper that drives a user-supplied tick callback each frame.
 * Counterpart to nq_action_tween_rect, but the user-supplied callback
 * usually interpolates an NqVec2f via nq_vec2f_lerp.
 *
 * Lifetime is identical to nq_action_tween_rect: caller supplies a tick
 * callback that knows how to advance its own state. When the callback
 * declares the animation finished (returns NQ_ACTION_FINISHED), the
 * action reports done and NqActionManager prunes it on the next tick.
 */
#ifndef NQ_ACTION_TWEEN_VEC2_H
#define NQ_ACTION_TWEEN_VEC2_H

#include <nq/action.h>

typedef struct {
    NqAction *action;
    void     *user;
} NqActionTweenVec2;

NqActionTweenVec2 *nq_action_tween_vec2_create(nq_action_tick_fn vec2_tick,
                                              void *user);

void nq_action_tween_vec2_destroy(NqActionTweenVec2 *t);
NqActionState nq_action_tween_vec2_update(NqActionTweenVec2 *t, float dt);
NqAction *nq_action_tween_vec2_action(NqActionTweenVec2 *t);

#endif /* NQ_ACTION_TWEEN_VEC2_H */
