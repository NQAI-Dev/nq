#include "nq/atlas.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    char  name[NQ_ATLAS_NAME_MAX];
    NqRect rect;
    int    alive;       /* 1 when active, 0 when removed (slot free) */
} NqAtlasRegion;

struct NqAtlas {
    int  width;
    int  height;
    size_t name_buf_len;
    size_t name_buf_used;

    NqAtlasRegion *regions;
    size_t         regions_len;   /* allocated slots */
    size_t         regions_count; /* live + dead = next free slot */

    /* The name arena. Region names copied into the first bytes; pointer
     * into the arena is the region's `name` field after construction. */
    char *name_buf;
};

/* Conservative default for the name arena. */
#define NQ_ATLAS_DEFAULT_NAME_BUF 4096

NqAtlas *nq_atlas_create(int width, int height, size_t name_buf_len) {
    NqAtlas *a = calloc(1, sizeof(NqAtlas));
    if (!a) return NULL;
    a->width = width;
    a->height = height;
    a->name_buf_len = name_buf_len ? name_buf_len : NQ_ATLAS_DEFAULT_NAME_BUF;
    a->name_buf = malloc(a->name_buf_len);
    if (!a->name_buf) {
        free(a);
        return NULL;
    }
    /* Allocate a small initial region slot set; grows on demand. */
    a->regions_len = 16;
    a->regions = calloc(a->regions_len, sizeof(NqAtlasRegion));
    if (!a->regions) {
        free(a->name_buf);
        free(a);
        return NULL;
    }
    return a;
}

void nq_atlas_destroy(NqAtlas *a) {
    if (!a) return;
    free(a->regions);
    free(a->name_buf);
    free(a);
}

int nq_atlas_width(const NqAtlas *a)  { return a ? a->width  : 0; }
int nq_atlas_height(const NqAtlas *a) { return a ? a->height : 0; }

/* Internal: grow the regions slot set if needed. */
static int nq_atlas_grow(NqAtlas *a) {
    if (a->regions_count < a->regions_len) return 0;
    size_t new_len = a->regions_len * 2;
    NqAtlasRegion *r = realloc(a->regions, new_len * sizeof(NqAtlasRegion));
    if (!r) return -1;
    memset(r + a->regions_len, 0, (new_len - a->regions_len) * sizeof(NqAtlasRegion));
    a->regions = r;
    a->regions_len = new_len;
    return 0;
}

int nq_atlas_add_region(NqAtlas *a, const char *name, NqRect rect) {
    if (!a || !name || name[0] == '\0') return -1;
    size_t name_len = strlen(name);
    if (name_len >= NQ_ATLAS_NAME_MAX) return -1;  /* bound per-name */
    if (a->name_buf_used + name_len + 1 > a->name_buf_len) return -1;

    /* Reject duplicates. */
    for (size_t i = 0; i < a->regions_count; i++) {
        if (a->regions[i].alive && strcmp(a->regions[i].name, name) == 0) {
            return -1;
        }
    }

    /* Reuse dead slots before allocating a new one. */
    size_t slot;
    for (slot = 0; slot < a->regions_count; slot++) {
        if (!a->regions[slot].alive) break;
    }
    if (slot == a->regions_count) {
        if (nq_atlas_grow(a) != 0) return -1;
        slot = a->regions_count;
        a->regions_count++;
    }

    /* Copy name into the arena. */
    memcpy(a->name_buf + a->name_buf_used, name, name_len + 1);
    /* Region's `name` field is a fixed-size array — we copy into it from
     * the arena rather than taking the pointer. */
    strncpy(a->regions[slot].name, a->name_buf + a->name_buf_used,
            NQ_ATLAS_NAME_MAX - 1);
    a->regions[slot].name[NQ_ATLAS_NAME_MAX - 1] = '\0';
    a->name_buf_used += name_len + 1;

    a->regions[slot].rect  = rect;
    a->regions[slot].alive = 1;
    return (int)slot;
}

int nq_atlas_remove_region(NqAtlas *a, const char *name) {
    if (!a || !name) return 0;
    for (size_t i = 0; i < a->regions_count; i++) {
        if (a->regions[i].alive && strcmp(a->regions[i].name, name) == 0) {
            a->regions[i].alive = 0;
            a->regions[i].name[0] = '\0';
            return 1;
        }
    }
    return 0;
}

NqRect nq_atlas_find(const NqAtlas *a, const char *name) {
    if (!a || !name) return nq_rect(0, 0, 0, 0);
    for (size_t i = 0; i < a->regions_count; i++) {
        if (a->regions[i].alive && strcmp(a->regions[i].name, name) == 0) {
            return a->regions[i].rect;
        }
    }
    return nq_rect(0, 0, 0, 0);
}

/* Iterator. Stores the next index to visit; iter_next returns 0 when the
 * iteration is exhausted. */
struct NqAtlasIter {
    const NqAtlas *a;
    size_t cursor;
};

NqAtlasIter *nq_atlas_iter_begin(const NqAtlas *a) {
    if (!a) return NULL;
    NqAtlasIter *it = calloc(1, sizeof(NqAtlasIter));
    if (!it) return NULL;
    it->a = a;
    it->cursor = 0;
    return it;
}

int nq_atlas_iter_next(NqAtlasIter *it, const char **name, NqRect *rect) {
    if (!it || !it->a) return 0;
    while (it->cursor < it->a->regions_count) {
        NqAtlasRegion *r = &it->a->regions[it->cursor++];
        if (r->alive) {
            if (name) *name = r->name;
            if (rect) *rect = r->rect;
            return 1;
        }
    }
    return 0;
}

void nq_atlas_iter_end(NqAtlasIter *it) {
    free(it);
}
