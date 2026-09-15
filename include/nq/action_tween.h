/*
 * nq — tween action.
 *
 * NqAction wrapper that drives a single NqAnimFloat from the action tick.
 * Composes nq_clock (dt source) + nq_tween (ease curve) + nq_animation
 * (float lerp primitive) into the action framework, so any
 * nq_action_tween instance can be scheduled from an action manager
 * alongside delays, sequences, etc. (those ship in later ticks).
 *
 * Lifetime: same rules as nq_action. Caller owns the struct. Destroy with
 * nq_action_tween_destroy (which also calls nq_action_destroy under the
 * hood — never call both).
 */
#ifndef NQ_ACTION_TWEEN_H
#define NQ_ACTION_TWEEN_H

#include <nq/action.h>
#include <nq/animation.h>

typedef struct {
    NqAction *action;
    NqAnimFloat *anim;       /* points into the user's storage */
} NqActionTween;

/* Build a tween action wrapping an existing NqAnimFloat. The action
 * ticks the animation each frame; when the animation completes, the
 * action returns FINISHED. The animation pointer must remain valid for
 * the action's lifetime (same lifetime contract as the action itself). */
NqActionTween *nq_action_tween_create(NqAnimFloat *anim);

/* Same as nq_action_destroy — destroys the underlying NqAction too. */
void nq_action_tween_destroy(NqActionTween *t);

/* Pass-through to nq_action_update. Returns the new state (RUNNING /
 * FINISHED / CANCELLED). */
NqActionState nq_action_tween_update(NqActionTween *t, float dt);

NqAction     *nq_action_tween_action(NqActionTween *t);

#endif /* NQ_ACTION_TWEEN_H */
