#include "nq/anim_vec2.h"

void nq_anim_vec2_init(NqAnimVec2 *a,
                       NqVec2f from, NqVec2f to,
                       float duration_seconds, NqEaseKind ease) {
    if (!a) return;
    a->from             = from;
    a->to               = to;
    a->duration_seconds = duration_seconds > 0.0f ? duration_seconds : 0.001f;
    a->elapsed_seconds  = 0.0f;
    a->active           = 1;
    a->reverse          = (to.x < from.x || to.y < from.y) ? 1 : 0;
    a->ease             = ease;
}

int nq_anim_vec2_update(NqAnimVec2 *a, float dt_seconds) {
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

int nq_anim_vec2_done(const NqAnimVec2 *a) {
    return a ? !a->active : 1;
}

NqVec2f nq_anim_vec2_value(const NqAnimVec2 *a) {
    if (!a) return nq_vec2f(0.0f, 0.0f);
    if (a->duration_seconds <= 0.0f) return a->to;
    float t = a->elapsed_seconds / a->duration_seconds;
    float eased = nq_ease(a->ease, t);
    return a->reverse
        ? nq_vec2f_lerp(a->to, a->from, eased)
        : nq_vec2f_lerp(a->from, a->to, eased);
}

void nq_anim_vec2_restart(NqAnimVec2 *a) {
    if (!a) return;
    a->elapsed_seconds = 0.0f;
    a->active = 1;
}
