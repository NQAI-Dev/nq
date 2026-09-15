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

NqAction *nq_action_create(nq_action_tick_fn tick,
                           nq_action_done_fn done,
                           void *user);
void      nq_action_destroy(NqAction *a);

/* Step the action forward by dt seconds. Returns the post-update state. */
NqActionState nq_action_update(NqAction *a, float dt);

/* Forced state transitions. */
void nq_action_cancel(NqAction *a);

/* Queries. */
NqActionState nq_action_state(const NqAction *a);
int           nq_action_is_finished(const NqAction *a);

#endif /* NQ_ACTION_H */
