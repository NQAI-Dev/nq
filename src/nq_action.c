#include "nq/action.h"

#include <stdlib.h>

struct NqAction {
    nq_action_tick_fn tick;
    nq_action_done_fn done;
    void             *user;
    NqActionState    state;
    int               done_fired;   /* 1 once `done` callback has run */
};

NqAction *nq_action_create(nq_action_tick_fn tick,
                           nq_action_done_fn done,
                           void *user) {
    if (!tick) return NULL;  /* tick is mandatory */
    NqAction *a = calloc(1, sizeof(NqAction));
    if (!a) return NULL;
    a->tick       = tick;
    a->done       = done;
    a->user       = user;
    a->state      = NQ_ACTION_RUNNING;
    a->done_fired = 0;
    return a;
}

void nq_action_destroy(NqAction *a) {
    free(a);
}

NqActionState nq_action_update(NqAction *a, float dt) {
    if (!a) return NQ_ACTION_FINISHED;
    if (a->state != NQ_ACTION_RUNNING) return a->state;
    a->state = a->tick(a, dt, a->user);
    if (a->state == NQ_ACTION_FINISHED && !a->done_fired && a->done) {
        a->done(a, a->user);
        a->done_fired = 1;
    }
    return a->state;
}

void nq_action_cancel(NqAction *a) {
    if (!a) return;
    if (a->state == NQ_ACTION_CANCELLED) return;
    a->state = NQ_ACTION_CANCELLED;
    if (!a->done_fired && a->done) {
        a->done(a, a->user);
        a->done_fired = 1;
    }
}

NqActionState nq_action_state(const NqAction *a) {
    return a ? a->state : NQ_ACTION_FINISHED;
}

int nq_action_is_finished(const NqAction *a) {
    if (!a) return 1;
    return a->state == NQ_ACTION_FINISHED || a->state == NQ_ACTION_CANCELLED;
}
