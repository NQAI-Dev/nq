/*
 * nq — fixed-cell spatial hash implementation.
 *
 * Internal layout
 * ---------------
 * Buckets: sh->buckets[bucket_count] — each bucket holds the index of the
 * first entry in a singly-linked chain, or NQ_SH_NONE (UINT32_MAX).
 *
 * Entries: sh->entries[max_entries] — a flat pool allocated once. Each entry
 * stores the object id, and the index of the next entry in the same bucket
 * chain. Pool allocation is a simple bump pointer (sh->entry_used); cleared
 * back to 0 by nq_spatial_hash_clear().
 *
 * Hash function: mix cell grid coordinates into a 32-bit hash, then modulo
 * bucket_count. A Knuth-style multiplicative hash avoids the worst-case
 * clustering of a naive (x*P + y) % N approach.
 */
#include "nq/spatial_hash.h"

#include <stdlib.h>
#include <string.h>

#define NQ_SH_NONE UINT32_MAX

typedef struct {
    uint32_t id;
    uint32_t next; /* next entry index in same bucket, or NQ_SH_NONE */
} NqSHEntry;

struct NqSpatialHash {
    int      cell_size;
    uint32_t bucket_count;
    uint32_t max_entries;
    uint32_t entry_used;  /* bump pointer into entries[] */

    uint32_t  *buckets; /* [bucket_count] head indices */
    NqSHEntry *entries; /* [max_entries]  entry pool   */
};

/* ---------------------------------------------------------------------------
 * Internal helpers
 * -------------------------------------------------------------------------*/

/* Map a world coordinate to its grid cell index (floor division). */
static inline int world_to_cell(int world, int cell_size) {
    /* C11 integer division truncates toward zero; we want floor. */
    if (world >= 0) {
        return world / cell_size;
    }
    /* For negative values: (world - cell_size + 1) / cell_size == floor. */
    return (world - cell_size + 1) / cell_size;
}

/* Hash a (cx, cy) cell coordinate pair to a bucket index. */
static inline uint32_t cell_hash(int cx, int cy, uint32_t bucket_count) {
    /* Pack into a 64-bit value and apply a multiplicative mix. */
    uint64_t h = ((uint64_t)(uint32_t)cx * 0x9e3779b97f4a7c15ULL) ^
                 ((uint64_t)(uint32_t)cy * 0x6c62272e07bb0142ULL);
    h ^= h >> 30;
    h *= 0xbf58476d1ce4e5b9ULL;
    h ^= h >> 27;
    return (uint32_t)(h % bucket_count);
}

/* ---------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------*/

NqSpatialHash *nq_spatial_hash_create(int cell_size,
                                      uint32_t bucket_count,
                                      uint32_t max_entries) {
    if (cell_size <= 0 || bucket_count == 0 || max_entries == 0) {
        return NULL;
    }

    NqSpatialHash *sh = (NqSpatialHash *)malloc(sizeof(*sh));
    if (!sh) return NULL;

    sh->buckets = (uint32_t *)malloc(sizeof(uint32_t) * bucket_count);
    if (!sh->buckets) {
        free(sh);
        return NULL;
    }

    sh->entries = (NqSHEntry *)malloc(sizeof(NqSHEntry) * max_entries);
    if (!sh->entries) {
        free(sh->buckets);
        free(sh);
        return NULL;
    }

    sh->cell_size    = cell_size;
    sh->bucket_count = bucket_count;
    sh->max_entries  = max_entries;
    sh->entry_used   = 0;

    /* Initialise all buckets to empty. */
    for (uint32_t i = 0; i < bucket_count; i++) {
        sh->buckets[i] = NQ_SH_NONE;
    }

    return sh;
}

void nq_spatial_hash_destroy(NqSpatialHash *sh) {
    if (!sh) return;
    free(sh->entries);
    free(sh->buckets);
    free(sh);
}

void nq_spatial_hash_clear(NqSpatialHash *sh) {
    if (!sh) return;
    sh->entry_used = 0;
    for (uint32_t i = 0; i < sh->bucket_count; i++) {
        sh->buckets[i] = NQ_SH_NONE;
    }
}

int nq_spatial_hash_insert(NqSpatialHash *sh, uint32_t id, NqRect aabb) {
    if (!sh) return 0;
    if (aabb.w <= 0 || aabb.h <= 0) return 0;

    int cs = sh->cell_size;

    /* Grid range covered by this AABB (inclusive on both ends). */
    int cx_min = world_to_cell(aabb.x,              cs);
    int cy_min = world_to_cell(aabb.y,              cs);
    /* aabb is half-open [x, x+w), so the last cell boundary is x+w-1. */
    int cx_max = world_to_cell(aabb.x + aabb.w - 1, cs);
    int cy_max = world_to_cell(aabb.y + aabb.h - 1, cs);

    int inserted = 0;

    for (int cy = cy_min; cy <= cy_max; cy++) {
        for (int cx = cx_min; cx <= cx_max; cx++) {
            if (sh->entry_used >= sh->max_entries) {
                /* Pool exhausted — drop silently (broadphase miss). */
                return inserted;
            }

            uint32_t bucket = cell_hash(cx, cy, sh->bucket_count);
            uint32_t idx    = sh->entry_used++;

            sh->entries[idx].id   = id;
            sh->entries[idx].next = sh->buckets[bucket];
            sh->buckets[bucket]   = idx;

            inserted++;
        }
    }

    return inserted;
}

NqSpatialHashQuery nq_spatial_hash_query_begin(const NqSpatialHash *sh,
                                               NqRect aabb) {
    NqSpatialHashQuery q;
    q.sh         = sh;
    q.seen_count = 0;

    if (!sh || aabb.w <= 0 || aabb.h <= 0) {
        /* Empty query: mark iteration as exhausted. */
        q.cell_x_min = 0; q.cell_x_max = -1;
        q.cell_y_min = 0; q.cell_y_max = -1;
        q.cell_x     = 0; q.cell_y     = 0;
        q.entry_idx  = NQ_SH_NONE;
        return q;
    }

    int cs = sh->cell_size;
    q.cell_x_min = world_to_cell(aabb.x,              cs);
    q.cell_y_min = world_to_cell(aabb.y,              cs);
    q.cell_x_max = world_to_cell(aabb.x + aabb.w - 1, cs);
    q.cell_y_max = world_to_cell(aabb.y + aabb.h - 1, cs);

    q.cell_x = q.cell_x_min;
    q.cell_y = q.cell_y_min;

    /* Point entry_idx at the head of the first bucket. */
    uint32_t bucket  = cell_hash(q.cell_x, q.cell_y, sh->bucket_count);
    q.entry_idx      = sh->buckets[bucket];

    return q;
}

int nq_spatial_hash_query_next(NqSpatialHashQuery *q, uint32_t *out_id) {
    if (!q || !out_id || !q->sh) return 0;

    const NqSpatialHash *sh = q->sh;

    for (;;) {
        /* Advance through the current bucket's chain. */
        while (q->entry_idx != NQ_SH_NONE) {
            uint32_t idx = q->entry_idx;
            uint32_t id  = sh->entries[idx].id;
            q->entry_idx = sh->entries[idx].next;

            /* Deduplication: skip ids already returned this query. */
            int seen = 0;
            for (int i = 0; i < q->seen_count; i++) {
                if (q->seen[i] == id) { seen = 1; break; }
            }
            if (seen) continue;

            /* Record in seen list (if room; overflow = tolerate duplicates). */
            if (q->seen_count < NQ_SPATIAL_HASH_QUERY_SEEN_MAX) {
                q->seen[q->seen_count++] = id;
            }

            *out_id = id;
            return 1;
        }

        /* Current cell exhausted — advance to the next cell. */
        q->cell_x++;
        if (q->cell_x > q->cell_x_max) {
            q->cell_x = q->cell_x_min;
            q->cell_y++;
        }
        if (q->cell_y > q->cell_y_max) {
            return 0; /* All cells visited. */
        }

        uint32_t bucket = cell_hash(q->cell_x, q->cell_y, sh->bucket_count);
        q->entry_idx    = sh->buckets[bucket];
    }
}

uint32_t nq_spatial_hash_entry_count(const NqSpatialHash *sh) {
    if (!sh) return 0;
    return sh->entry_used;
}
