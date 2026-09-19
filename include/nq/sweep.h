/*
 * nq — sweep-and-prune (SAP) broadphase collision pair enumeration.
 *
 * Complements nq/spatial_hash.h: where the spatial hash answers
 * "which objects overlap this AABB?", sweep-and-prune answers
 * "give me ALL overlapping pairs in the current set" in one call —
 * the classic per-physics-step broadphase. Sorts object AABBs by
 * min-x, then sweeps the sorted axis: each object is tested only
 * against the run of successors whose min-x is still below its
 * max-x, with the y-interval test pruning the rest. Near-linear
 * for spreads-out scenes; degrades toward O(n^2) only when huge
 * x-overlapping clusters exist (same failure mode as any SAP).
 *
 * Design constraints
 * ------------------
 * - Pure C11, no SDL3 or other external deps.
 * - Object ids are caller-owned uint32_t, same convention as the
 *   spatial hash: no pointers, lifetime stays with the caller.
 * - Fixed capacity chosen at create(); overflow drops silently
 *   (insert returns 0), matching the spatial hash philosophy.
 * - Half-open AABB convention like the rest of nq: two boxes that
 *   share only an edge do NOT pair (strict < on both axes).
 * - Thread-safety: none. One NqSweep per physics world.
 *
 * Typical per-frame usage
 * -----------------------
 *   nq_sweep_clear(sw);
 *   for (int i = 0; i < n_objects; i++)
 *       nq_sweep_insert(sw, ids[i], aabbs[i].x, aabbs[i].y,
 *                       aabbs[i].x + aabbs[i].w, aabbs[i].y + aabbs[i].h);
 *   nq_sweep_for_each_pair(sw, on_broadphase_pair, &world);
 *   // on_broadphase_pair(id_a, id_b, user) runs the narrow phase.
 */
#ifndef NQ_SWEEP_H
#define NQ_SWEEP_H

#include <stdint.h>

/* Opaque handle. */
typedef struct NqSweep NqSweep;

/*
 * Pair callback. Invoked once per overlapping pair (id_a, id_b) with
 * id_a earlier in the x-sweep than id_b (i.e. min_x(a) <= min_x(b);
 * for equal min-x the order is unspecified). Never invoked with
 * id_a == id_b unless the caller inserted the same id twice at
 * overlapping positions — duplicate ids are the caller's business.
 */
typedef void (*NqSweepPairFn)(uint32_t id_a, uint32_t id_b, void *user);

/*
 * Allocate a sweep set.
 *
 *   max_objects — capacity: maximum number of live objects between
 *                 clear() calls. Must be > 0.
 *
 * Returns NULL on invalid arguments or allocation failure.
 */
NqSweep *nq_sweep_create(uint32_t max_objects);

/* Free all memory. sw may be NULL. */
void nq_sweep_destroy(NqSweep *sw);

/*
 * Remove all objects. O(1). Does not free memory.
 * Call once per frame before re-inserting all objects.
 */
void nq_sweep_clear(NqSweep *sw);

/*
 * Insert object `id` with the axis-aligned bounding box
 * (min_x, min_y) — (max_x, max_y) in half-open coordinates.
 * Degenerate boxes (max <= min on either axis) are accepted but can
 * never produce pairs.
 *
 * Returns 1 on success, 0 when the set is full (insert dropped).
 */
int nq_sweep_insert(NqSweep *sw, uint32_t id,
                    float min_x, float min_y,
                    float max_x, float max_y);

/*
 * Sort by min-x, sweep the axis, and invoke `fn(id_a, id_b, user)`
 * for every pair whose AABBs overlap on BOTH axes. fn may be NULL —
 * then this just counts (useful for testing / budgeting).
 *
 * May reorder the internal storage; pair content is unaffected.
 * Returns the number of overlapping pairs found.
 */
uint32_t nq_sweep_for_each_pair(NqSweep *sw, NqSweepPairFn fn, void *user);

/* Diagnostic: number of objects currently inserted. */
uint32_t nq_sweep_count(const NqSweep *sw);

#endif /* NQ_SWEEP_H */
