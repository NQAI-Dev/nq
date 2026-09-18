#include <nq/action_tween_vec2.h>
#include <stdlib.h>

static NqActionState nq_action_tween_vec2_tick(NqAction *a, float dt, void *userdata) {
    (void)a;
    NqActionTweenVec2 *t = (NqActionTweenVec2 *)userdata;
    if (t->custom_tick) {
        NqActionState res = t->custom_tick(a, dt, t->custom_user);
        if (res == NQ_ACTION_FINISHED) {
            return NQ_ACTION_FINISHED;
        }
    }
    if (t->anim) {
        if (!t->anim->active) {
            return NQ_ACTION_FINISHED;
        }
        nq_anim_vec2_update(t->anim, dt);
        if (nq_anim_vec2_done(t->anim)) {
            return NQ_ACTION_FINISHED;
        }
    }
    return NQ_ACTION_RUNNING;
}

NqActionTweenVec2 *nq_action_tween_vec2_create(nq_action_tick_fn tick, void *user) {
    NqActionTweenVec2 *t = (NqActionTweenVec2 *)malloc(sizeof(NqActionTweenVec2));
    if (!t) return NULL;
    t->anim = NULL;
    t->custom_tick = tick;
    t->custom_user = user;
    t->action = nq_action_create(nq_action_tween_vec2_tick, NULL, t);
    if (!t->action) {
        free(t);
        return NULL;
    }
    return t;
}

void nq_action_tween_vec2_set_anim(NqActionTweenVec2 *t, NqAnimVec2 *anim) {
    if (t) t->anim = anim;
}

void nq_action_tween_vec2_destroy(NqActionTweenVec2 *t) {
    if (!t) return;
    nq_action_destroy(t->action);
    free(t);
}

NqActionState nq_action_tween_vec2_update(NqActionTweenVec2 *t, float dt) {
    if (!t || !t->action) return NQ_ACTION_CANCELLED;
    return nq_action_update(t->action, dt);
}

NqAction *nq_action_tween_vec2_action(NqActionTweenVec2 *t) {
    if (!t) return NULL;
    return t->action;
}
