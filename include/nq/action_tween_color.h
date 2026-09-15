/*
 * nq — color tween action.
 *
 * NqAction wrapper that drives a single NqAnimColor from the action tick.
 * Composes NqClock (dt source) + NqTween (ease curve) + NqColor (lerp
 * primitive) + NqAction (state machine) into the action framework, so
 * NqAnimColor instances can be scheduled via NqActionManager.
 *
 * Lifetime: same rules as NqActionTween — caller-owned struct, paired
 * with an existing NqAnimColor. Destroy with nq_action_tween_color_destroy
 * (which also calls nq_action_destroy under the hood — never call both).
 */
#ifndef NQ_ACTION_TWEEN_COLOR_H
#define NQ_ACTION_TWEEN_COLOR_H

#include <nq/action.h>
#include <nq/anim_color.h>

typedef struct {
    NqAction    *action;
    NqAnimColor *anim;       /* points into the user's storage */
} NqActionTweenColor;

/* Build a tween action wrapping an existing NqAnimColor. The action
 * ticks the animation each frame; when the animation completes, the
 * action returns FINISHED. */
NqActionTweenColor *nq_action_tween_color_create(NqAnimColor *anim);

/* Same as nq_action_destroy — destroys the underlying NqAction too. */
void nq_action_tween_color_destroy(NqActionTweenColor *t);

/* Pass-through to nq_action_update. Returns the new state. */
NqActionState nq_action_tween_color_update(NqActionTweenColor *t, float dt);

NqAction *nq_action_tween_color_action(NqActionTweenColor *t);

#endif /* NQ_ACTION_TWEEN_COLOR_H */
