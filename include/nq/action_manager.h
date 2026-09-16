/*
 * nq — action manager.
 *
 * Owns a flat list of NqAction pointers and ticks them once per frame.
 * Finished or cancelled actions are removed from the list automatically so
 * the caller never has to deal with the lifecycle bookkeeping.
 *
 * Lifetime: the manager does NOT own the actions it ticks. Callers pass
 * in NqAction*; once an action finishes it is dropped from the manager's
 * internal list but the caller still owns the pointer (and is expected to
 * destroy it).
 *
 * Capacity: small fixed upper bound (NqActionManagerMax = 256) for
 * now — games don't usually have hundreds of concurrent actions. If a
 * project needs more, swap the storage to a heap array without changing
 * the public API.
 */
#ifndef NQ_ACTION_MANAGER_H
#define NQ_ACTION_MANAGER_H

#include <stddef.h>

#include "nq/action.h"

#define NQ_ACTION_MANAGER_MAX 256

typedef struct NqActionManager NqActionManager;

NqActionManager *nq_action_manager_create(void);
void             nq_action_manager_destroy(NqActionManager *m);

/* Add an action. Returns 1 if added, 0 if the manager is full or m is
 * NULL. Caller still owns the action; manager does not destroy it on
 * removal (it just stops tracking it). */
int nq_action_manager_add(NqActionManager *m, NqAction *a);

/* Remove an action by pointer (linear search; the cap is small). The
 * action is not destroyed — caller still owns it. Returns 1 if removed,
 * 0 if not found. */
int nq_action_manager_remove(NqActionManager *m, NqAction *a);

/* Tick every active action by dt. Finished/cancelled actions are removed
 * from the list. Returns the number of actions that completed on this
 * call (useful for "all done?" checks). */
int nq_action_manager_tick(NqActionManager *m, float dt);

/* Clear all actions from the manager (without destroying them). */
void nq_action_manager_clear(NqActionManager *m);

/* Queries. */
size_t nq_action_manager_count(const NqActionManager *m);

/* Returns the maximum number of actions the manager can track (the
 * fixed upper bound set at compile time as NQ_ACTION_MANAGER_MAX).
 * Useful for callers that want to check capacity before add() — saves
 * a trial-and-error add() that would return 0 only at saturation. */
int nq_action_manager_capacity(void);

#endif /* NQ_ACTION_MANAGER_H */
