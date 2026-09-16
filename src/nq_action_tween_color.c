#include "nq/action_tween_color.h"
#include "nq/log.h"

#include <stdlib.h>

static NqActionState color_tween_tick(NqAction *a, float dt, void *user) {
    (void)a;
    NqAnimColor *anim = (NqAnimColor *)user;
    if (!anim) return NQ_ACTION_FINISHED;
    if (nq_anim_color_done(anim)) return NQ_ACTION_FINISHED;
    nq_anim_color_update(anim, dt);
    return nq_anim_color_done(anim)
        ? NQ_ACTION_FINISHED
        : NQ_ACTION_RUNNING;
}

NqActionTweenColor *nq_action_tween_color_create(NqAnimColor *anim) {
    if (!anim) return NULL;
    NqActionTweenColor *t = calloc(1, sizeof(NqActionTweenColor));
    if (!t) return NULL;
    t->anim = anim;
    t->action = nq_action_create(color_tween_tick, NULL, anim);
    if (!t->action) {
        free(t);
        return NULL;
    }
    return t;
}

void nq_action_tween_color_destroy(NqActionTweenColor *t) {
    if (!t) return;
    if (t->action) nq_action_destroy(t->action);
    free(t);
}

NqActionState nq_action_tween_color_update(NqActionTweenColor *t, float dt) {
    if (!t || !t->action) return NQ_ACTION_FINISHED;
    return nq_action_update(t->action, dt);
}

NqAction *nq_action_tween_color_action(NqActionTweenColor *t) {
    return t ? t->action : NULL;
}
