#include "nq/action_manager.h"

#include <stdlib.h>

struct NqActionManager {
    NqAction *actions[NQ_ACTION_MANAGER_MAX];
    size_t    count;
};

NqActionManager *nq_action_manager_create(void) {
    NqActionManager *m = calloc(1, sizeof(NqActionManager));
    return m;
}

void nq_action_manager_destroy(NqActionManager *m) {
    /* Manager does NOT own the actions; just free the manager struct. */
    free(m);
}

int nq_action_manager_add(NqActionManager *m, NqAction *a) {
    if (!m || !a) return 0;
    if (m->count >= NQ_ACTION_MANAGER_MAX) return 0;
    /* Reject duplicates — same pointer tracked twice would tick twice and
     * complete twice. */
    for (size_t i = 0; i < m->count; i++) {
        if (m->actions[i] == a) return 0;
    }
    m->actions[m->count++] = a;
    return 1;
}

int nq_action_manager_remove(NqActionManager *m, NqAction *a) {
    if (!m || !a) return 0;
    for (size_t i = 0; i < m->count; i++) {
        if (m->actions[i] == a) {
            /* Compact: move last to current, drop the tail. Order doesn't
             * matter for the tick walk; we always iterate linearly. */
            m->actions[i] = m->actions[m->count - 1];
            m->actions[m->count - 1] = NULL;
            m->count--;
            return 1;
        }
    }
    return 0;
}

int nq_action_manager_tick(NqActionManager *m, float dt) {
    if (!m) return 0;
    int completed_this_call = 0;
    /* Walk forward; remove in place by swapping the trailing slot down
     * when an action terminates. Indices are evaluated in a stable way
     * because new completed actions only shrink the loop range. */
    size_t i = 0;
    while (i < m->count) {
        NqAction *a = m->actions[i];
        NqActionState s = nq_action_update(a, dt);
        if (s != NQ_ACTION_RUNNING) {
            /* Remove by swap-with-last, decrement count. */
            m->actions[i] = m->actions[m->count - 1];
            m->actions[m->count - 1] = NULL;
            m->count--;
            completed_this_call++;
            /* Don't increment i — the swapped action needs ticking
             * this same frame. */
            continue;
        }
        i++;
    }
    return completed_this_call;
}

void nq_action_manager_clear(NqActionManager *m) {
    if (!m) return;
    for (size_t i = 0; i < m->count; i++) {
        m->actions[i] = NULL;
    }
    m->count = 0;
}

size_t nq_action_manager_count(const NqActionManager *m) {
    return m ? m->count : 0;
}
