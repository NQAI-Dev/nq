/*
 * nq — rect tween action.
 *
 * NqAction wrapper that drives a user-supplied tick callback each frame.
 * Composes the action framework with the NqRect lerp primitive so rect
 * animations (sizing, position, both) compose with NqActionManager,
 * NqActionSequence, NqActionSpawn, etc.
 *
 * Lifecycle is identical to nq_action_tween_color: caller supplies a
 * tick callback that knows how to advance its own state (e.g. lerp a
 * NqRect toward a target). When the callback declares the animation
 * finished (returns NQ_ACTION_FINISHED), the action reports done and
 * NqActionManager prunes it on the next tick.
 */
#ifndef NQ_ACTION_TWEEN_RECT_H
#define NQ_ACTION_TWEEN_RECT_H

#include <nq/action.h>

typedef struct {
    NqAction *action;
    void     *user;
} NqActionTweenRect;

/* Build a rect-tween action wrapping the user's tick callback.
 * `rect_tick` is called per frame and returns the new action state.
 * `user` is passed verbatim to the callback. */
NqActionTweenRect *nq_action_tween_rect_create(nq_action_tick_fn rect_tick,
                                             void *user);

void nq_action_tween_rect_destroy(NqActionTweenRect *t);
NqActionState nq_action_tween_rect_update(NqActionTweenRect *t, float dt);
NqAction *nq_action_tween_rect_action(NqActionTweenRect *t);

#endif /* NQ_ACTION_TWEEN_RECT_H */
