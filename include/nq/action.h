/*
 * nq — action primitive.
 *
 * Lightweight stateful per-frame step function. The engine calls
 * nq_action_update(a, dt) every frame; the action's tick callback advances
 * its internal state and returns NQ_ACTION_RUNNING or NQ_ACTION_FINISHED.
 * Caller code composes higher-level constructs (sequences, parallels,
 * tweens, delays) on top.
 *
 * Actions don't run their own tick — the engine drives them via
 * nq_action_update(). This keeps timing deterministic (the engine's
 * clock is the only time source).
 */
#ifndef NQ_ACTION_H
#define NQ_ACTION_H

typedef enum {
    NQ_ACTION_RUNNING = 0,   /* keep stepping */
    NQ_ACTION_FINISHED,      /* remove from active set */
    NQ_ACTION_CANCELLED,     /* removed by external cancel */
} NqActionState;

typedef struct NqAction NqAction;

/* Per-frame step. The callback advances its internal state and returns
 * the new action state. dt is in seconds (matches nq_clock_tick). */
typedef NqActionState (*nq_action_tick_fn)(NqAction *a, float dt, void *user);

/* Optional: called exactly once when the action reaches NQ_ACTION_FINISHED
 * (whether by the tick callback or by an external cancel). May be NULL. */
typedef void (*nq_action_done_fn)(NqAction *a, void *user);

/* Optional: re-arm the action so the next update() ticks from the
 * beginning again. Takes the action's user pointer (rather than the
 * action itself) so composition primitives can install a reset_fn
 * without exposing NqAction's internal struct layout.
 *
 * NULL means there is no action-specific state to reset; the base action
 * state can still be re-armed by nq_action_reset. */
typedef void (*nq_action_reset_fn)(void *user);

NqAction *nq_action_create(nq_action_tick_fn tick,
                           nq_action_done_fn done,
                           void *user);
void *nq_action_get_user(NqAction *a);
void      nq_action_destroy(NqAction *a);

/* Step the action forward by dt seconds. Returns the post-update state. */
NqActionState nq_action_update(NqAction *a, float dt);

/* Forced state transitions. */
void nq_action_cancel(NqAction *a);

/* Install a reset callback on a freshly-created action. Composition
 * primitives call this from their constructors to expose reset
 * semantics; user-defined actions built on top of nq_action_create
 * can use it too. Pass NULL to remove an existing reset callback. */
void nq_action_set_reset(NqAction *a, nq_action_reset_fn reset);

/* Re-arm the action by putting it back into RUNNING state. If a reset_fn
 * is installed, also call it to zero action-specific counters.
 *
 * Note: nq_action_reset itself does NOT trigger the action's done()
 * callback; it just reverses a FINISHED transition. The callback had
 * already fired when the action originally terminated. */
void nq_action_reset(NqAction *a);

/* Queries. */
NqActionState nq_action_state(const NqAction *a);
int           nq_action_is_finished(const NqAction *a);

#endif /* NQ_ACTION_H */
