/*
 * nq — delay / wait action.
 *
 * Counts elapsed time and reports FINISHED once duration is reached.
 * Tick callback is pure CPU work — no callbacks, no allocations per
 * frame. Composable with other actions (sequence: delay + tween).
 */
#ifndef NQ_ACTION_DELAY_H
#define NQ_ACTION_DELAY_H

#include <nq/action.h>

typedef struct {
    NqAction *action;
    float duration_seconds;
    float elapsed_seconds;
} NqActionDelay;

NqActionDelay *nq_action_delay_create(float duration_seconds);
void           nq_action_delay_destroy(NqActionDelay *d);
NqActionState  nq_action_delay_update(NqActionDelay *d, float dt);
NqAction      *nq_action_delay_action(NqActionDelay *d);
float          nq_action_delay_elapsed(const NqActionDelay *d);
float          nq_action_delay_duration(const NqActionDelay *d);

#endif /* NQ_ACTION_DELAY_H */
