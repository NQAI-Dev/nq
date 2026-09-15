/*
 * nq — scene-stack wrapper.
 *
 * A scene owns one NqNode tree (the "current root") and supports a stack
 * of trees. update/draw target the top of the stack so menus and pause
 * overlays can push their own trees without disturbing the active game
 * scene.
 *
 * Memory: scenes are independent — push/pop are pure pointer swaps on the
 * internal stack, no tree copies. When the top scene is popped and the
 * previous scene comes back, its tree is exactly as it was.
 *
 * The scene does NOT take ownership of trees beyond the implicit
 * `current_root` it creates in nq_scene_create. Caller-created trees
 * passed via nq_scene_push remain owned by whoever created them; the
 * caller must destroy them (or hand them to a scene that takes
 * ownership — future tick if needed).
 */
#ifndef NQ_SCENE_H
#define NQ_SCENE_H

#include <stddef.h>
#include "nq/node.h"

#define NQ_SCENE_STACK_MAX 16

typedef struct NqScene NqScene;

NqScene *nq_scene_create(void);
void     nq_scene_destroy(NqScene *s);

NqNode  *nq_scene_root(NqScene *s);  /* the implicit root tree, always present */

/* Push a tree onto the stack as the new active root. If the stack is
 * full, returns 0 and the tree is left untouched. Ownership of `tree`
 * stays with the caller. */
int  nq_scene_push(NqScene *s, NqNode *tree);

/* Pop the current top tree off the stack. If only the implicit root
 * remains (stack depth 1), returns NULL — call sites can rely on this to
 * detect "we're at the bottom". The popped tree is NOT destroyed; caller
 * owns it. */
NqNode *nq_scene_pop(NqScene *s);

NqNode  *nq_scene_top(NqScene *s);   /* top of stack without popping */
size_t   nq_scene_depth(NqScene *s);

/* Per-frame. update walks the top subtree; draw does the same. The
 * implicit root is always updated (it usually holds shared systems like
 * the renderer / global input) but only drawn if its visible flag is set
 * so a bare scene doesn't draw twice. */
void nq_scene_update(NqScene *s, float dt);
void nq_scene_draw(NqScene *s);

#endif /* NQ_SCENE_H */
