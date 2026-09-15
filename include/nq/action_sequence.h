#include <stddef.h>
#include <stddef.h>

/*
 * nq — sequence action.
 *
 * Runs a fixed list of NqActions one after another. The sequence reports
 * RUNNING while at least one sub-action is RUNNING, and FINISHED when
 * all of them have terminated (FINISHED or CANCELLED).
 *
 * Lifetime: the sequence owns its sub-actions — when the sequence is
 * destroyed (or reaches FINISHED through cancel), it destroys each
 * sub-action it was holding. Caller transfers ownership by adding
 * actions to the sequence.
 *
 * Allocation: zero dynamic memory beyond the storage for the action
 * pointers (fixed array, NQ_ACTION_SEQUENCE_MAX = 16). A real game
 * rarely has more than a handful of sequential actions; if a project
 * needs more, swap to a heap array.
 */
#ifndef NQ_ACTION_SEQUENCE_H
#define NQ_ACTION_SEQUENCE_H

#include <nq/action.h>

#define NQ_ACTION_SEQUENCE_MAX 16

typedef struct {
    NqAction *action;
    NqAction *sub_actions[NQ_ACTION_SEQUENCE_MAX];
    size_t    sub_count;
    int       current;  /* index of the active sub-action */
    int       owns_subs; /* 1: destroy sub-actions when sequence finishes */
} NqActionSequence;

/* Build a sequence from an array of NQ_ACTION_SEQUENCE_MAX-sub-action
 * list. NUL-terminated? No — caller passes count. */
NqActionSequence *nq_action_sequence_create(const NqAction **subs,
                                            size_t count,
                                            int take_ownership);

/* Add a single sub-action. Returns 1 on success, 0 if full / NULL. */
int  nq_action_sequence_add(NqActionSequence *s, NqAction *a, int take_ownership);

void          nq_action_sequence_destroy(NqActionSequence *s);
NqActionState nq_action_sequence_update(NqActionSequence *s, float dt);
NqAction     *nq_action_sequence_action(NqActionSequence *s);
int           nq_action_sequence_is_done(const NqActionSequence *s);
size_t        nq_action_sequence_sub_count(const NqActionSequence *s);
int           nq_action_sequence_current_index(const NqActionSequence *s);

#endif /* NQ_ACTION_SEQUENCE_H */
