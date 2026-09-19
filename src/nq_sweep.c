/*
 * nq — sweep-and-prune broadphase implementation.
 *
 * Storage model: two arrays allocated once at create().
 *   items[]  — the object list, kept in arbitrary order between
 *              clear()/insert calls; sorted by min_x at the start of
 *              every for_each_pair sweep.
 *   The sort is a plain insertion sort (qsort's callback indirection
 *   buys nothing at the typical <few-thousand objects scale, and a
 *              stable in-place insertion sort keeps equal-min_x order
 *              deterministic across platforms — no libc comparison
 *              quirks). O(n^2) worst-case on sort matches the worst
 *              case of the sweep itself, so no asymptotic loss.
 *
 * Pair enumeration: classic single-axis sweep. After sorting by
 * min_x, for each item i walk j > i while items[j].min_x <
 * items[i].max_x; report the pair only when the y-intervals also
 * overlap (strictly, half-open like the rest of nq).
 */
#include "nq/sweep.h"

#include <stdlib.h>

typedef struct {
    uint32_t id;
    float    min_x;
    float    min_y;
    float    max_x;
    float    max_y;
} NqSweepItem;

struct NqSweep {
    uint32_t     capacity;
    uint32_t     count;
    NqSweepItem *items;
};

NqSweep *nq_sweep_create(uint32_t max_objects) {
    if (max_objects == 0) {
        return NULL;
    }
    NqSweep *sw = calloc(1, sizeof(*sw));
    if (sw == NULL) {
        return NULL;
    }
    sw->capacity = max_objects;
    sw->count = 0;
    /* calloc for the item array: guarantees a defined all-zero state
     * even before the first clear(). */
    sw->items = calloc(max_objects, sizeof(*sw->items));
    if (sw->items == NULL) {
        free(sw);
        return NULL;
    }
    return sw;
}

void nq_sweep_destroy(NqSweep *sw) {
    if (sw == NULL) {
        return;
    }
    free(sw->items);
    free(sw);
}

void nq_sweep_clear(NqSweep *sw) {
    if (sw == NULL) {
        return;
    }
    sw->count = 0;
}

int nq_sweep_insert(NqSweep *sw, uint32_t id,
                    float min_x, float min_y,
                    float max_x, float max_y) {
    if (sw == NULL || sw->count >= sw->capacity) {
        return 0;
    }
    NqSweepItem *it = &sw->items[sw->count++];
    it->id    = id;
    it->min_x = min_x;
    it->min_y = min_y;
    it->max_x = max_x;
    it->max_y = max_y;
    return 1;
}

/* Insertion sort by min_x, ascending. Stable, deterministic. */
static void sweep_sort_by_min_x(NqSweepItem *items, uint32_t n) {
    for (uint32_t i = 1; i < n; i++) {
        NqSweepItem key = items[i];
        uint32_t j = i;
        while (j > 0 && items[j - 1].min_x > key.min_x) {
            items[j] = items[j - 1];
            j--;
        }
        items[j] = key;
    }
}

uint32_t nq_sweep_for_each_pair(NqSweep *sw, NqSweepPairFn fn, void *user) {
    if (sw == NULL || sw->count < 2) {
        return 0;
    }
    sweep_sort_by_min_x(sw->items, sw->count);

    uint32_t pairs = 0;
    for (uint32_t i = 0; i < sw->count; i++) {
        const NqSweepItem *a = &sw->items[i];
        for (uint32_t j = i + 1; j < sw->count; j++) {
            const NqSweepItem *b = &sw->items[j];
            /* Sweep axis prune: once b starts after a ends, every later
             * item starts even later — break the inner loop. */
            if (b->min_x >= a->max_x) {
                break;
            }
            /* Cross-axis test: strict half-open overlap on y too. */
            if (b->min_y < a->max_y && a->min_y < b->max_y) {
                pairs++;
                if (fn != NULL) {
                    fn(a->id, b->id, user);
                }
            }
        }
    }
    return pairs;
}

uint32_t nq_sweep_count(const NqSweep *sw) {
    return sw == NULL ? 0 : sw->count;
}
