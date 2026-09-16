#include "nq/action_repeat.h"

#include <stdlib.h>

static NqActionState repeat_tick(NqAction *a, float dt, void *user) {
    (void)a;
    NqActionRepeat *r = (NqActionRepeat *)user;
    if (!r || !r->sub) return NQ_ACTION_FINISHED;

    /* Tick the sub. If it finished and we have iterations left, restart
     * it. If infinite, always restart. If count exhausted, FINISHED. */
    NqActionState sub_state = nq_action_update(r->sub, dt);
    if (sub_state == NQ_ACTION_RUNNING) {
        return NQ_ACTION_RUNNING;
    }
    /* Sub terminated — should we restart? */
    if (r->infinite) {
        /* The user's tick callback returns RUNNING on re-entry after a
         * previous FINISHED (counter_tick does this; a pure "play clip"
         * action doesn't). So infinite mode just keeps the wrapper
         * RUNNING and trusts the sub. */
        return NQ_ACTION_RUNNING;
    }
    /* Finite mode: decrement counter. When it hits 0 we stop. */
    if (r->remaining > 0) r->remaining--;
    if (r->remaining <= 0) return NQ_ACTION_FINISHED;
    return NQ_ACTION_RUNNING;
}

/* Restore the iteration counter to the value supplied at construction.
 * For infinite mode the "original count" is -1 (forever) so reset
 * leaves it at -1. */
static void repeat_reset(void *user) {
    NqActionRepeat *r = (NqActionRepeat *)user;
    if (r) r->remaining = r->original_count;
}

NqActionRepeat *nq_action_repeat_create(NqAction *sub, int times) {
    if (!sub || times < 0) return NULL;
    NqActionRepeat *r = calloc(1, sizeof(NqActionRepeat));
    if (!r) return NULL;
    r->sub = sub;
    r->remaining = times;
    r->original_count = times;
    r->infinite = 0;
    r->action = nq_action_create(repeat_tick, NULL, r);
    if (!r->action) {
        free(r);
        return NULL;
    }
    nq_action_set_reset(r->action, repeat_reset);
    return r;
}

NqActionRepeat *nq_action_repeat_forever_create(NqAction *sub) {
    if (!sub) return NULL;
    NqActionRepeat *r = calloc(1, sizeof(NqActionRepeat));
    if (!r) return NULL;
    r->sub = sub;
    r->remaining = -1;
    r->original_count = -1;
    r->infinite = 1;
    r->action = nq_action_create(repeat_tick, NULL, r);
    if (!r->action) {
        free(r);
        return NULL;
    }
    nq_action_set_reset(r->action, repeat_reset);
    return r;
}

void nq_action_repeat_destroy(NqActionRepeat *r) {
    if (!r) return;
    if (r->action) nq_action_destroy(r->action);
    /* Don't destroy sub — caller owns it (mirror sequence behaviour). */
    free(r);
}

NqActionState nq_action_repeat_update(NqActionRepeat *r, float dt) {
    if (!r || !r->action) return NQ_ACTION_FINISHED;
    return nq_action_update(r->action, dt);
}

NqAction *nq_action_repeat_action(NqActionRepeat *r) {
    return r ? r->action : NULL;
}

int nq_action_repeat_remaining(const NqActionRepeat *r) {
    return r ? r->remaining : 0;
}

int nq_action_repeat_total(const NqActionRepeat *r) {
    /* Returns the iteration count this repeat was created with (-1 for
     * forever). Useful for UIs that want to show "iteration N of M";
     * call nq_action_repeat_remaining for N and total() for M. */
    return r ? r->original_count : 0;
}
