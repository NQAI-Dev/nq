/*
 * nq — repeat action.
 *
 * Wraps a single sub-action and re-runs it N times (or forever with
 * nq_action_repeat_forever). On each restart the sub-action must be in
 * FINISHED state for the wrapper to know to begin a new iteration.
 * The wrapper never destroys the sub — caller retains ownership.
 *
 * Count limit: nq_action_repeat_create takes an explicit iteration
 * count (0 → FINISHED immediately). For unbounded, use
 * nq_action_repeat_forever_create which sets a "no count" flag.
 *
 * Capacity: 1 sub per repeat wrapper (a repeat of a sequence = a
 * sequence of repeating actions; no need to nest multiple repeats
 * inside one wrapper).
 */
#ifndef NQ_ACTION_REPEAT_H
#define NQ_ACTION_REPEAT_H

#include <nq/action.h>

typedef struct {
    NqAction *action;
    NqAction *sub;
    int       remaining;   /* iterations left; -1 = infinite */
    int       infinite;
} NqActionRepeat;

NqActionRepeat *nq_action_repeat_create(NqAction *sub, int times);
NqActionRepeat *nq_action_repeat_forever_create(NqAction *sub);

void          nq_action_repeat_destroy(NqActionRepeat *r);
NqActionState nq_action_repeat_update(NqActionRepeat *r, float dt);
NqAction     *nq_action_repeat_action(NqActionRepeat *r);
int           nq_action_repeat_remaining(const NqActionRepeat *r);

#endif /* NQ_ACTION_REPEAT_H */
