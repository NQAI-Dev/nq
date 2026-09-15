#include "nq/anim_color.h"
#include "nq/log.h"

#include <math.h>

void nq_anim_color_init(NqAnimColor *a,
                       NqColor from, NqColor to,
                       float duration_seconds, NqEaseKind ease) {
    if (!a) return;
    a->from             = from;
    a->to               = to;
    a->duration_seconds = duration_seconds > 0.0f
                          ? duration_seconds : 0.001f;
    a->elapsed_seconds  = 0.0f;
    a->active           = 1;
    a->reverse          = (to.r < from.r || to.g < from.g ||
                           to.b < from.b || to.a < from.a) ? 1 : 0;
    a->ease             = ease;
}

int nq_anim_color_update(NqAnimColor *a, float dt_seconds) {
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

int nq_anim_color_done(const NqAnimColor *a) {
    return a ? !a->active : 1;
}

NqColor nq_anim_color_value(const NqAnimColor *a) {
    if (!a) return NQ_COLOR_RGB(0, 0, 0);
    if (a->duration_seconds <= 0.0f) return a->to;
    float t = a->elapsed_seconds / a->duration_seconds;
    float eased = nq_ease(a->ease, t);
    /* Like NqAnimFloat: when reverse direction, walk from `to` to `from`
     * using (1 - eased) so the eased curve still drives smoothly in
     * either direction. */
    return a->reverse
        ? nq_color_lerp(a->to, a->from, eased)
        : nq_color_lerp(a->from, a->to, eased);
}

void nq_anim_color_restart(NqAnimColor *a) {
    if (!a) return;
    a->elapsed_seconds = 0.0f;
    a->active = 1;
}
