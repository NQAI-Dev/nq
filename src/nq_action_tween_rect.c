#include <nq/action_tween_rect.h>
#include <stdlib.h>

static NqActionState nq_action_tween_rect_tick(NqAction *a, float dt, void *userdata) {
    (void)a;
    NqActionTweenRect *t = (NqActionTweenRect *)userdata;
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
        nq_anim_rect_update(t->anim, dt);
        if (nq_anim_rect_done(t->anim)) {
            return NQ_ACTION_FINISHED;
        }
    }
    return NQ_ACTION_RUNNING;
}

NqActionTweenRect *nq_action_tween_rect_create(nq_action_tick_fn tick, void *user) {
    NqActionTweenRect *t = (NqActionTweenRect *)malloc(sizeof(NqActionTweenRect));
    if (!t) return NULL;
    t->anim = NULL;
    t->custom_tick = tick;
    t->custom_user = user;
    t->action = nq_action_create(nq_action_tween_rect_tick, NULL, t);
    if (!t->action) {
        free(t);
        return NULL;
    }
    return t;
}

void nq_action_tween_rect_set_anim(NqActionTweenRect *t, NqAnimRect *anim) {
    if (t) t->anim = anim;
}

void nq_action_tween_rect_destroy(NqActionTweenRect *t) {
    if (!t) return;
    nq_action_destroy(t->action);
    free(t);
}

NqActionState nq_action_tween_rect_update(NqActionTweenRect *t, float dt) {
    if (!t || !t->action) return NQ_ACTION_FINISHED;
    return nq_action_update(t->action, dt);
}

NqAction *nq_action_tween_rect_action(NqActionTweenRect *t) {
    if (!t) return NULL;
    return t->action;
}
