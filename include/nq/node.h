/*
 * nq — scene-graph node.
 *
 * Phase 3 core. A node is a tree element with local transform, optional
 * user callbacks (init/update/draw), and 0+ children. The engine walks the
 * tree from any root to call update/draw on live nodes; transforms are
 * accumulated by walking the parent chain so a child's world_pos is
 * parent.world(parent.local(child.local)).
 *
 * Memory: nodes own their name (interned on the heap) and their children
 * list. nq_node_destroy() recursively destroys the subtree. Engines hold
 * a single root node per scene and treat it as owned by the scene.
 */
#ifndef NQ_NODE_H
#define NQ_NODE_H

#include <stddef.h>

#define NQ_NODE_NAME_MAX 32

typedef struct NqNode NqNode;

typedef void (*nq_node_init_fn)  (NqNode *n, void *user);
typedef void (*nq_node_update_fn)(NqNode *n, float dt, void *user);
typedef void (*nq_node_draw_fn)  (NqNode *n, void *user);

typedef enum {
    NQ_NODE_FLAG_ALIVE = 0x1,    /* not yet destroyed */
    NQ_NODE_FLAG_VISIBLE = 0x2,  /* subtree is rendered */
    NQ_NODE_FLAG_UPDATING = 0x4, /* subtree receives update() */
} NqNodeFlags;

NqNode *nq_node_create(NqNode *parent, const char *name);
void     nq_node_destroy(NqNode *n);  /* destroys the entire subtree */

/* Reparenting. Re-parents `child` under `new_parent`. If new_parent is
 * NULL, the child becomes a root. Caller must guarantee there is no
 * cycle (child is not an ancestor of new_parent). */
void nq_node_attach(NqNode *child, NqNode *new_parent);

/* Tree iteration. Returns NULL when iteration is exhausted. */
NqNode *nq_node_parent(const NqNode *n);
NqNode *nq_node_first_child(const NqNode *n);
NqNode *nq_node_next_sibling(const NqNode *n);

/* Flags (NqNodeFlags). Default after create: ALIVE | VISIBLE | UPDATING. */
void nq_node_set_flag(NqNode *n, NqNodeFlags flag, int on);
int  nq_node_get_flag(NqNode *n, NqNodeFlags flag);

/* Local transform: position/rotation/scale. World transform is computed
 * lazily via nq_node_world_*() — no caching for now (cheap on small
 * trees; caching is a future tick if profiling demands). */
void nq_node_set_position(NqNode *n, float x, float y);
void nq_node_set_rotation(NqNode *n, float degrees);
void nq_node_set_scale(NqNode *n, float sx, float sy);
void nq_node_get_position(const NqNode *n, float *out_x, float *out_y);
float nq_node_get_rotation(const NqNode *n);
void nq_node_get_scale(const NqNode *n, float *out_sx, float *out_sy);

/* Optional per-node callbacks. user is passed verbatim to each callback. */
void nq_node_set_callbacks(NqNode *n,
                           nq_node_init_fn init,
                           nq_node_update_fn update,
                           nq_node_draw_fn draw,
                           void *user);

/* Subtree walks. For each live node in pre-order: init (once, on first
 * update call) then update(dt) at each frame; draw traverses the same
 * subtree. The "first update" hook fires init on nodes that haven't been
 * initialized yet — convenient for one-shot setup. */
void nq_node_update(NqNode *root, float dt);
void nq_node_draw(NqNode *root);

#endif /* NQ_NODE_H */
