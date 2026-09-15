/*
 * nq — spawn action (parallel composite).
 *
 * Runs multiple sub-actions in parallel. Reports RUNNING while at least
 * one sub-action is RUNNING; reports FINISHED when all have terminated
 * (whether by their own tick returning non-RUNNING, or by external
 * cancel). The complement of nq_action_sequence (which runs serially).
 *
 * Lifetime: same rules as nq_action_sequence — caller transfers
 * ownership via take_ownership=1, sequence destroys sub-actions on
 * its own destroy.
 *
 * Capacity: NQ_ACTION_SPAWN_MAX = 16 parallel subs. Real games rarely
 * need more than a handful of true parallel actions (animations,
 * particles, multi-input waits); if more is needed, swap to a heap
 * array without touching the public API.
 */
#ifndef NQ_ACTION_SPAWN_H
#define NQ_ACTION_SPAWN_H

#include <nq/action.h>

#define NQ_ACTION_SPAWN_MAX 16

typedef struct {
    NqAction *action;
    NqAction *sub_actions[NQ_ACTION_SPAWN_MAX];
    size_t    sub_count;
    int       owns_subs;
    int       finished_subs;  /* count of subs that have terminated */
} NqActionSpawn;

NqActionSpawn *nq_action_spawn_create(const NqAction **subs, size_t count,
                                     int take_ownership);
int            nq_action_spawn_add(NqActionSpawn *s, NqAction *a,
                                  int take_ownership);

void          nq_action_spawn_destroy(NqActionSpawn *s);
NqActionState nq_action_spawn_update(NqActionSpawn *s, float dt);
NqAction     *nq_action_spawn_action(NqActionSpawn *s);
int           nq_action_spawn_is_done(const NqActionSpawn *s);
size_t        nq_action_spawn_sub_count(const NqActionSpawn *s);
int           nq_action_spawn_finished_sub_count(const NqActionSpawn *s);

#endif /* NQ_ACTION_SPAWN_H */
