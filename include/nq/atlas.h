/*
 * nq — texture atlas region manager.
 *
 * Atlas as data: a (width, height) bounding box plus a string-keyed map
 * of named rectangular regions (frames, tiles, sprite parts). The texture
 * itself is *not* owned by the atlas — wire it up at draw time via the
 * helper that wraps both NqAtlas + NqTexture (added in a follow-up tick).
 *
 * Region names are interned at add-time: copies of the input string are
 * owned by the atlas and freed on remove / destroy. Looking up by
 * string is O(n) — atlas sizes in real games are tens to hundreds of
 * entries and a hash table buys nothing measurable. If a single atlas
 * ever needs thousands of regions, swap to a hash map without changing
 * the public API.
 */
#ifndef NQ_ATLAS_H
#define NQ_ATLAS_H

#include <nq/common.h>
#include <nq/rect.h>

#define NQ_ATLAS_NAME_MAX 32

typedef struct NqAtlas NqAtlas;

/* Construct an empty atlas. (width, height) is the bounding box of the
 * underlying texture (informational — the atlas doesn't allocate a
 * texture). name_buf_len is the total bytes available for the name
 * string arena; pass 0 for the default (4 KiB). */
NqAtlas *nq_atlas_create(int width, int height, size_t name_buf_len);
void     nq_atlas_destroy(NqAtlas *a);

int      nq_atlas_width(const NqAtlas *a);
int      nq_atlas_height(const NqAtlas *a);

/* Add a region. The name is copied into the atlas' internal name arena.
 * Returns the region index (>=0) on success, -1 on allocation failure
 * or duplicate name. */
int      nq_atlas_add_region(NqAtlas *a, const char *name, NqRect rect);

/* Remove a region by name. Returns 1 if removed, 0 if not found. */
int      nq_atlas_remove_region(NqAtlas *a, const char *name);

/* Look up a region by name. Returns the rect on success, or an empty
 * rect (w<=0 || h<=0) when the region is not found. The returned rect
 * is owned by the atlas and valid until the region is removed or the
 * atlas destroyed. */
NqRect   nq_atlas_find(const NqAtlas *a, const char *name);

/* Draw a region through a paired NqTexture. Looks up the named region
 * in the atlas, then forwards to nq_texture_draw_region. Returns 0 on
 * success, -1 if either handle is NULL, the region doesn't exist, or the
 * underlying SDL3 call failed. */
int nq_atlas_draw(const NqAtlas *atlas, struct NqTexture *tex,
                 const char *region_name, int dst_x, int dst_y);

/* Forward declaration so atlas.h doesn't have to pull in texture.h
 * itself; consumers include both. */
struct NqTexture;

/* Iteration. Returns NULL when iteration is exhausted. */
typedef struct NqAtlasIter NqAtlasIter;
NqAtlasIter *nq_atlas_iter_begin(const NqAtlas *a);
int          nq_atlas_iter_next(NqAtlasIter *it,
                                const char **name, NqRect *rect);
void         nq_atlas_iter_end(NqAtlasIter *it);

#endif /* NQ_ATLAS_H */
