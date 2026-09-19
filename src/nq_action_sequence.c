#include "nq/action_sequence.h"
#include "nq/log.h"

#include <stdlib.h>

static NqActionState seq_tick(NqAction *a, float dt, void *user) {
    (void)a;
    NqActionSequence *s = (NqActionSequence *)user;
    if (!s) return NQ_ACTION_FINISHED;

    /* Fast-path: if all subs are already done, sequence is finished. */
    if (s->current >= (int)s->sub_count) return NQ_ACTION_FINISHED;

    /* Tick the currently-active sub-action. If it finished, advance. */
    NqActionState state = nq_action_update(s->sub_actions[s->current], dt);
    if (state != NQ_ACTION_RUNNING) {
        s->current++;
        if (s->current >= (int)s->sub_count) {
            return NQ_ACTION_FINISHED;
        }
        /* The next sub-action is now the active one — return RUNNING
         * so the caller continues ticking us next frame. */
        return NQ_ACTION_RUNNING;
    }
    return NQ_ACTION_RUNNING;
}

static void seq_reset(void *user) {
    /* Restart the sequence from the first sub-action. */
    NqActionSequence *s = (NqActionSequence *)user;
    if (s) s->current = 0;
}

NqActionSequence *nq_action_sequence_create(NqAction **subs,
                                            size_t count,
                                            int take_ownership) {
    if (!subs && count > 0) return NULL;
    if (count > NQ_ACTION_SEQUENCE_MAX) return NULL;

    NqActionSequence *s = calloc(1, sizeof(NqActionSequence));
    if (!s) return NULL;
    s->owns_subs = take_ownership ? 1 : 0;
    s->current = 0;

    if (count > 0) {
        s->sub_count = count;
        for (size_t i = 0; i < count; i++) {
            s->sub_actions[i] = subs[i];
        }
    }
    s->action = nq_action_create(seq_tick, NULL, s);
    nq_action_set_reset(s->action, seq_reset);
    if (!s->action) {
        free(s);
        return NULL;
    }
    return s;
}

int nq_action_sequence_add(NqActionSequence *s, NqAction *a, int take_ownership) {
    if (!s || !a) return 0;
    if (s->sub_count >= NQ_ACTION_SEQUENCE_MAX) return 0;
    /* If ownership flag changes mid-sequence, honour the latest setting. */
    if (take_ownership) s->owns_subs = 1;
    s->sub_actions[s->sub_count++] = a;
    return 1;
}

void nq_action_sequence_destroy(NqActionSequence *s) {
    if (!s) return;
    if (s->owns_subs) {
        for (size_t i = 0; i < s->sub_count; i++) {
            if (s->sub_actions[i]) nq_action_destroy(s->sub_actions[i]);
        }
    }
    if (s->action) nq_action_destroy(s->action);
    free(s);
}

NqActionState nq_action_sequence_update(NqActionSequence *s, float dt) {
    if (!s || !s->action) return NQ_ACTION_FINISHED;
    return nq_action_update(s->action, dt);
}

NqAction *nq_action_sequence_action(NqActionSequence *s) {
    return s ? s->action : NULL;
}

int nq_action_sequence_is_done(const NqActionSequence *s) {
    if (!s) return 1;
    return s->current >= (int)s->sub_count;
}

size_t nq_action_sequence_sub_count(const NqActionSequence *s) {
    return s ? s->sub_count : 0;
}

int nq_action_sequence_current_index(const NqActionSequence *s) {
    return s ? s->current : -1;
}
