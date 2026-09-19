#include "nq/action_spawn.h"
#include "nq/log.h"

#include <stdlib.h>

static NqActionState spawn_tick(NqAction *a, float dt, void *user) {
    (void)a;
    NqActionSpawn *s = (NqActionSpawn *)user;
    if (!s) return NQ_ACTION_FINISHED;

    /* Tick every still-running sub-action. Count how many have finished
     * so far (cumulative across ticks — we don't reset, since a finished
     * sub stays finished). */
    int newly_finished_this_call = 0;
    for (size_t i = 0; i < s->sub_count; i++) {
        if (!s->sub_actions[i]) continue;
        NqActionState sub_state = nq_action_update(s->sub_actions[i], dt);
        if (sub_state != NQ_ACTION_RUNNING) {
            /* Update finished_subs by re-counting (cheap: NQ_ACTION_SPAWN_MAX
             * is bounded, and tracking per-slot state would need another
             * array). Linear re-count keeps state consistent without
             * duplicating the membership table. */
            int live = 0;
            for (size_t j = 0; j < s->sub_count; j++) {
                if (s->sub_actions[j] &&
                    nq_action_state(s->sub_actions[j]) == NQ_ACTION_RUNNING) {
                    live++;
                }
            }
            s->finished_subs = (int)s->sub_count - live;
        }
        (void)newly_finished_this_call;
    }

    return s->finished_subs >= (int)s->sub_count
        ? NQ_ACTION_FINISHED
        : NQ_ACTION_RUNNING;
}

NqActionSpawn *nq_action_spawn_create(NqAction **subs, size_t count,
                                     int take_ownership) {
    if (!subs && count > 0) return NULL;
    if (count > NQ_ACTION_SPAWN_MAX) return NULL;

    NqActionSpawn *s = calloc(1, sizeof(NqActionSpawn));
    if (!s) return NULL;
    s->owns_subs    = take_ownership ? 1 : 0;
    s->sub_count    = count;
    s->finished_subs = 0;

    if (count > 0) {
        for (size_t i = 0; i < count; i++) {
            s->sub_actions[i] = subs[i];
        }
    }
    s->action = nq_action_create(spawn_tick, NULL, s);
    if (!s->action) {
        free(s);
        return NULL;
    }
    return s;
}

int nq_action_spawn_add(NqActionSpawn *s, NqAction *a, int take_ownership) {
    if (!s || !a) return 0;
    if (s->sub_count >= NQ_ACTION_SPAWN_MAX) return 0;
    if (take_ownership) s->owns_subs = 1;
    s->sub_actions[s->sub_count++] = a;
    return 1;
}

void nq_action_spawn_destroy(NqActionSpawn *s) {
    if (!s) return;
    if (s->owns_subs) {
        for (size_t i = 0; i < s->sub_count; i++) {
            if (s->sub_actions[i]) nq_action_destroy(s->sub_actions[i]);
        }
    }
    if (s->action) nq_action_destroy(s->action);
    free(s);
}

NqActionState nq_action_spawn_update(NqActionSpawn *s, float dt) {
    if (!s || !s->action) return NQ_ACTION_FINISHED;
    return nq_action_update(s->action, dt);
}

NqAction *nq_action_spawn_action(NqActionSpawn *s) {
    return s ? s->action : NULL;
}

int nq_action_spawn_is_done(const NqActionSpawn *s) {
    if (!s) return 1;
    return s->finished_subs >= (int)s->sub_count;
}

size_t nq_action_spawn_sub_count(const NqActionSpawn *s) {
    return s ? s->sub_count : 0;
}

int nq_action_spawn_finished_sub_count(const NqActionSpawn *s) {
    return s ? s->finished_subs : 0;
}
