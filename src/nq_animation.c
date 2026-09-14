#include "nq/animation.h"

#define NQ_ANIM_MIN_DURATION_SECONDS 0.001f

void nq_anim_float_init(NqAnimFloat *a,
                        float from, float to,
                        float duration_seconds,
                        NqEaseKind ease) {
    if (!a) return;
    a->ease             = ease;
    a->duration_seconds = duration_seconds > 0.0f
                          ? duration_seconds
                          : NQ_ANIM_MIN_DURATION_SECONDS;
    a->elapsed_seconds  = 0.0f;
    a->from             = from;
    a->to               = to;
    a->active           = 1;
    a->reverse          = (to < from) ? 1 : 0;
}

int nq_anim_float_update(NqAnimFloat *a, float dt_seconds) {
    if (!a) return 0;
    if (!a->active) return 0;
    if (dt_seconds < 0.0f) dt_seconds = 0.0f;
    a->elapsed_seconds += dt_seconds;
    int just_completed = 0;
    if (a->elapsed_seconds >= a->duration_seconds) {
        a->elapsed_seconds = a->duration_seconds;
        a->active = 0;
        just_completed = 1;
    }
    return just_completed;
}

int nq_anim_float_done(const NqAnimFloat *a) {
    return a ? !a->active : 1;  /* NULL anim is treated as "done" */
}

float nq_anim_float_value(const NqAnimFloat *a) {
    if (!a) return 0.0f;
    if (a->duration_seconds <= 0.0f) return a->to;
    float t = a->elapsed_seconds / a->duration_seconds;
    float eased = nq_ease(a->ease, t);
    /* When reverse direction is requested (to < from), the eased curve
     * still walks 0..1 from from toward to — the caller's "from > to" is
     * still achieved because nq_ease_lerp-style interpolation is here done
     * inside this function via a->reverse. */
    float base = a->reverse
        ? (a->to + (a->from - a->to) * (1.0f - eased))
        : (a->from + (a->to - a->from) * eased);
    return base;
}

void nq_anim_float_restart(NqAnimFloat *a) {
    if (!a) return;
    a->elapsed_seconds = 0.0f;
    a->active = 1;
}
