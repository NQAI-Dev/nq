#include "nq/action_tween_vec2.h"

#include <stdlib.h>

static NqActionState vec2_tick_dispatch(NqAction *a, float dt, void *user) {
    nq_action_tick_fn user_tick = (nq_action_tick_fn)user;
    if (!user_tick) return NQ_ACTION_FINISHED;
    return user_tick(a, dt, NULL);
}

NqActionTweenVec2 *nq_action_tween_vec2_create(nq_action_tick_fn vec2_tick,
                                              void *user) {
    if (!vec2_tick) return NULL;
    NqActionTweenVec2 *t = calloc(1, sizeof(NqActionTweenVec2));
    if (!t) return NULL;
    t->user = user;
    t->action = nq_action_create(vec2_tick_dispatch, NULL, vec2_tick);
    if (!t->action) {
        free(t);
        return NULL;
    }
    return t;
}

void nq_action_tween_vec2_destroy(NqActionTweenVec2 *t) {
    if (!t) return;
    if (t->action) nq_action_destroy(t->action);
    free(t);
}

NqActionState nq_action_tween_vec2_update(NqActionTweenVec2 *t, float dt) {
    if (!t || !t->action) return NQ_ACTION_FINISHED;
    return nq_action_update(t->action, dt);
}

NqAction *nq_action_tween_vec2_action(NqActionTweenVec2 *t) {
    return t ? t->action : NULL;
}
