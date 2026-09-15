#include "nq/action_delay.h"

#include <stdlib.h>

static NqActionState delay_tick(NqAction *a, float dt, void *user) {
    (void)a;
    NqActionDelay *d = (NqActionDelay *)user;
    if (!d) return NQ_ACTION_FINISHED;
    d->elapsed_seconds += dt;
    if (d->elapsed_seconds >= d->duration_seconds) {
        d->elapsed_seconds = d->duration_seconds;
        return NQ_ACTION_FINISHED;
    }
    return NQ_ACTION_RUNNING;
}

NqActionDelay *nq_action_delay_create(float duration_seconds) {
    if (duration_seconds < 0.0f) duration_seconds = 0.0f;
    NqActionDelay *d = calloc(1, sizeof(NqActionDelay));
    if (!d) return NULL;
    d->duration_seconds = duration_seconds;
    d->elapsed_seconds = 0.0f;
    d->action = nq_action_create(delay_tick, NULL, d);
    if (!d->action) {
        free(d);
        return NULL;
    }
    /* Zero-duration delay finishes immediately on first tick — caller's
     * contract: any subsequent tick returns FINISHED. */
    if (duration_seconds == 0.0f) {
        /* Force finished state by advancing elapsed past duration. */
        d->elapsed_seconds = 0.0f;
    }
    return d;
}

void nq_action_delay_destroy(NqActionDelay *d) {
    if (!d) return;
    if (d->action) nq_action_destroy(d->action);
    free(d);
}

NqActionState nq_action_delay_update(NqActionDelay *d, float dt) {
    if (!d || !d->action) return NQ_ACTION_FINISHED;
    return nq_action_update(d->action, dt);
}

NqAction *nq_action_delay_action(NqActionDelay *d) {
    return d ? d->action : NULL;
}

float nq_action_delay_elapsed(const NqActionDelay *d) {
    return d ? d->elapsed_seconds : 0.0f;
}

float nq_action_delay_duration(const NqActionDelay *d) {
    return d ? d->duration_seconds : 0.0f;
}
