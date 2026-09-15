#include "nq/action_tween.h"

#include <stdlib.h>

static NqActionState tween_tick(NqAction *a, float dt, void *user) {
    (void)a;
    NqAnimFloat *anim = (NqAnimFloat *)user;
    if (!anim) return NQ_ACTION_FINISHED;
    if (nq_anim_float_done(anim)) return NQ_ACTION_FINISHED;
    nq_anim_float_update(anim, dt);
    return nq_anim_float_done(anim)
        ? NQ_ACTION_FINISHED
        : NQ_ACTION_RUNNING;
}

static void tween_reset(void *user) {
    NqAnimFloat *anim = (NqAnimFloat *)user;
    if (anim) nq_anim_float_restart(anim);
}

NqActionTween *nq_action_tween_create(NqAnimFloat *anim) {
    if (!anim) return NULL;
    NqActionTween *t = calloc(1, sizeof(NqActionTween));
    if (!t) return NULL;
    t->anim = anim;
    t->action = nq_action_create(tween_tick, NULL, anim);
    nq_action_set_reset(t->action, tween_reset);
    if (!t->action) {
        free(t);
        return NULL;
    }
    return t;
}

void nq_action_tween_destroy(NqActionTween *t) {
    if (!t) return;
    if (t->action) nq_action_destroy(t->action);
    free(t);
}

NqActionState nq_action_tween_update(NqActionTween *t, float dt) {
    if (!t || !t->action) return NQ_ACTION_FINISHED;
    return nq_action_update(t->action, dt);
}

NqAction *nq_action_tween_action(NqActionTween *t) {
    return t ? t->action : NULL;
}
