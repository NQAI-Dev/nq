/*
 * nq — fixed-cell spatial hash for broadphase collision queries.
 *
 * Maps 2D world-space integer positions to a flat hash table of object ids.
 * Each cell covers a square region of `cell_size` units. An object inserted
 * at a world-space AABB (x, y, w, h) is registered in every cell whose
 * region overlaps that AABB — the broadphase query then returns candidate
 * pairs without the O(n²) all-pairs scan.
 *
 * Design constraints
 * ------------------
 * - Pure C11, no SDL3 or other external deps.
 * - No dynamic allocation after nq_spatial_hash_create(); the internal
 *   bucket array and entry pool are allocated once and reused across
 *   nq_spatial_hash_clear() calls.
 * - Object ids are caller-owned unsigned ints (uint32_t). The hash table
 *   does not hold pointers — ownership and lifetime stay with the caller.
 * - Fixed capacity (max_entries across ALL cells): callers choose at
 *   creation time based on their expected world size and density.
 * - Thread-safety: none. Callers that parallelize physics must serialize
 *   access or use separate hash instances.
 *
 * Typical per-frame usage
 * -----------------------
 *   nq_spatial_hash_clear(sh);
 *   for (int i = 0; i < n_objects; i++)
 *       nq_spatial_hash_insert(sh, ids[i], aabbs[i]);
 *   NqSpatialHashQuery q = nq_spatial_hash_query_begin(sh, query_aabb);
 *   uint32_t candidate;
 *   while (nq_spatial_hash_query_next(&q, &candidate))
 *       maybe_collide(my_id, candidate);
 */
#ifndef NQ_SPATIAL_HASH_H
#define NQ_SPATIAL_HASH_H

#include <stddef.h>
#include <stdint.h>

#include <nq/rect.h>

/* Opaque handle. */
typedef struct NqSpatialHash NqSpatialHash;

/*
 * Query cursor returned by nq_spatial_hash_query_begin(). Stack-allocated
 * by the caller; no teardown needed — just stop calling query_next().
 * Deduplication: an id that appears in multiple overlapping cells is
 * returned only once per query (tracked via a small inline seen-list;
 * if the seen-list overflows the caller may receive duplicates, which is
 * acceptable for broadphase — the narrow phase re-checks anyway).
 */
#define NQ_SPATIAL_HASH_QUERY_SEEN_MAX 64

typedef struct {
    const NqSpatialHash *sh;
    /* Iteration state over the cells covered by the query rect. */
    int cell_x;      /* current grid X being visited */
    int cell_y;      /* current grid Y being visited */
    int cell_x_min;
    int cell_x_max;
    int cell_y_min;
    int cell_y_max;
    /* Position within the current cell's entry chain. */
    uint32_t entry_idx; /* index into sh->entries[], NQ_SH_NONE = end */
    /* Deduplication: ids already returned this query. */
    uint32_t seen[NQ_SPATIAL_HASH_QUERY_SEEN_MAX];
    int      seen_count;
} NqSpatialHashQuery;

/*
 * Allocate a spatial hash.
 *
 *   cell_size   — width (and height) of each grid cell in world units.
 *                 Must be > 0.
 *   bucket_count — number of hash buckets. Power-of-two recommended for
 *                  performance; the implementation works for any value > 0.
 *   max_entries  — total entry slots across all cells. If inserting an
 *                  object would overflow this limit, the insert is silently
 *                  dropped (broadphase misses are recoverable; crashes
 *                  are not).
 *
 * Returns NULL on invalid arguments or allocation failure.
 */
NqSpatialHash *nq_spatial_hash_create(int cell_size,
                                      uint32_t bucket_count,
                                      uint32_t max_entries);

/* Free all memory. sh may be NULL. */
void nq_spatial_hash_destroy(NqSpatialHash *sh);

/*
 * Remove all entries. O(bucket_count). Does not free memory.
 * Call once per frame before re-inserting all objects.
 */
void nq_spatial_hash_clear(NqSpatialHash *sh);

/*
 * Insert object `id` into every cell touched by `aabb`.
 * Empty rects (w<=0 || h<=0) are ignored.
 * Returns the number of cells the object was inserted into (0 on overflow
 * or empty rect).
 */
int nq_spatial_hash_insert(NqSpatialHash *sh, uint32_t id, NqRect aabb);

/*
 * Begin a query for all objects whose AABB overlaps `aabb`.
 * Returns a cursor; call nq_spatial_hash_query_next() to iterate results.
 */
NqSpatialHashQuery nq_spatial_hash_query_begin(const NqSpatialHash *sh,
                                               NqRect aabb);

/*
 * Advance the query cursor. Writes the next candidate id into *out_id and
 * returns 1. Returns 0 when no more candidates exist.
 * Duplicates are suppressed up to NQ_SPATIAL_HASH_QUERY_SEEN_MAX unique ids.
 */
int nq_spatial_hash_query_next(NqSpatialHashQuery *q, uint32_t *out_id);

/* Diagnostic: number of entry slots currently used (across all cells). */
uint32_t nq_spatial_hash_entry_count(const NqSpatialHash *sh);

#endif /* NQ_SPATIAL_HASH_H */
