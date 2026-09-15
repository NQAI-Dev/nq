#include "nq_action_repeat.h"

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
        /* No API to "restart" an NqAction without recreating it. The
         * user's tick callback however can return RUNNING again from the
         * start, which is the same as restarting the sub for our
         * purposes. Since we can't reset internal state here, we just
         * keep RUNNING and let the user's tick keep firing (a no-op
         * for one frame, then it'll keep returning RUNNING).
         *
         * But this breaks the "one sub-action that restarts N times"
         * contract for infinite mode. A cleaner design would have
         * nq_action_reset(sub) — future tick. For now, mark this with a
         * note: infinite mode requires the sub's tick to be re-callable. */
        return NQ_ACTION_RUNNING;
    }
    /* Finite mode: decrement counter, restart sub by ticking it again.
     * Same caveat: without reset, the sub's tick needs to behave
     * well when called after FINISHED — that's a sub-by-sub concern
     * (e.g. counter_tick returns RUNNING until target). */
    if (r->remaining > 0) r->remaining--;
    if (r->remaining <= 0) return NQ_ACTION_FINISHED;
    return NQ_ACTION_RUNNING;
}

NqActionRepeat *nq_action_repeat_create(NqAction *sub, int times) {
    if (!sub || times < 0) return NULL;
    NqActionRepeat *r = calloc(1, sizeof(NqActionRepeat));
    if (!r) return NULL;
    r->sub = sub;
    r->remaining = times;
    r->infinite = 0;
    r->action = nq_action_create(repeat_tick, NULL, r);
    if (!r->action) {
        free(r);
        return NULL;
    }
    return r;
}

NqActionRepeat *nq_action_repeat_forever_create(NqAction *sub) {
    if (!sub) return NULL;
    NqActionRepeat *r = calloc(1, sizeof(NqActionRepeat));
    if (!r) return NULL;
    r->sub = sub;
    r->remaining = -1;
    r->infinite = 1;
    r->action = nq_action_create(repeat_tick, NULL, r);
    if (!r->action) {
        free(r);
        return NULL;
    }
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
    if (!r) return 0;
    return r->remaining;
}
