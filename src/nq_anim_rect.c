#include "nq/anim_rect.h"

void nq_anim_rect_init(NqAnimRect *a,
                      NqRect from, NqRect to,
                      float duration_seconds, NqEaseKind ease) {
    if (!a) return;
    a->from             = from;
    a->to               = to;
    a->duration_seconds = duration_seconds > 0.0f ? duration_seconds : 0.001f;
    a->elapsed_seconds  = 0.0f;
    a->active           = 1;
    a->reverse          = (to.x < from.x || to.y < from.y ||
                            to.w < from.w || to.h < from.h) ? 1 : 0;
    a->ease             = ease;
}

int nq_anim_rect_update(NqAnimRect *a, float dt_seconds) {
    if (!a) return 0;
    if (!a->active) return 0;
    if (dt_seconds < 0.0f) dt_seconds = 0.0f;
    a->elapsed_seconds += dt_seconds;
    if (a->elapsed_seconds >= a->duration_seconds) {
        a->elapsed_seconds = a->duration_seconds;
        a->active = 0;
        return 1;
    }
    return 0;
}

int nq_anim_rect_done(const NqAnimRect *a) {
    return a ? !a->active : 1;
}

NqRect nq_anim_rect_value(const NqAnimRect *a) {
    if (!a) return nq_rect(0, 0, 0, 0);
    if (a->duration_seconds <= 0.0f) return a->to;
    float t = a->elapsed_seconds / a->duration_seconds;
    float eased = nq_ease(a->ease, t);
    return a->reverse
        ? nq_rect_lerp(a->to, a->from, eased)
        : nq_rect_lerp(a->from, a->to, eased);
}

void nq_anim_rect_restart(NqAnimRect *a) {
    if (!a) return;
    a->elapsed_seconds = 0.0f;
    a->active = 1;
}
