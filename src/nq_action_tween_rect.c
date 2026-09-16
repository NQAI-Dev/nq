#include "nq/action_tween_rect.h"

#include <stdlib.h>

static NqActionState rect_tick_dispatch(NqAction *a, float dt, void *user) {
    nq_action_tick_fn user_tick = (nq_action_tick_fn)user;
    if (!user_tick) return NQ_ACTION_FINISHED;
    return user_tick(a, dt, a->user);
}

NqActionTweenRect *nq_action_tween_rect_create(nq_action_tick_fn rect_tick,
                                             void *user) {
    if (!rect_tick) return NULL;
    NqActionTweenRect *t = calloc(1, sizeof(NqActionTweenRect));
    if (!t) return NULL;
    t->user = user;
    /* The outer wrapper's tick looks like a normal action tick (returns
     * NqActionState, signature matches nq_action_tick_fn). The user's
     * actual tick callback is invoked with the user's own user pointer
     * (passed through via NqAction's user). We pass `rect_tick` itself
     * as the user pointer at the outer wrapper level so the outer
     * wrapper can dispatch the right inner callback. */
    t->action = nq_action_create(rect_tick_dispatch, NULL, rect_tick);
    if (!t->action) {
        free(t);
        return NULL;
    }
    return t;
}

void nq_action_tween_rect_destroy(NqActionTweenRect *t) {
    if (!t) return;
    if (t->action) nq_action_destroy(t->action);
    /* Don't free `user` — caller owns it. */
    free(t);
}

NqActionState nq_action_tween_rect_update(NqActionTweenRect *t, float dt) {
    if (!t || !t->action) return NQ_ACTION_FINISHED;
    return nq_action_update(t->action, dt);
}

NqAction *nq_action_tween_rect_action(NqActionTweenRect *t) {
    return t ? t->action : NULL;
}
