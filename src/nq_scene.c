#include "nq/scene.h"
#include "nq/log.h"

#include <stdlib.h>

struct NqScene {
    NqNode *stack[NQ_SCENE_STACK_MAX];
    size_t  depth;
};

/* Allocates and creates an implicit root tree. Always present — even if
 * the caller never pushes, update() still runs over the implicit root
 * so globally-attached systems can tick. */
NqScene *nq_scene_create(void) {
    NqScene *s = calloc(1, sizeof(NqScene));
    if (!s) {
        NQ_LOG_ERROR("nq_scene_create: out of memory");
        return NULL;
    }
    s->stack[0] = nq_node_create(NULL, "scene_root");
    if (!s->stack[0]) {
        free(s);
        return NULL;
    }
    s->depth = 1;
    return s;
}

void nq_scene_destroy(NqScene *s) {
    if (!s) return;
    /* Implicit root is owned by us; destroy it. */
    if (s->stack[0]) {
        nq_node_destroy(s->stack[0]);
    }
    free(s);
}

NqNode *nq_scene_root(NqScene *s) {
    return (s && s->depth > 0) ? s->stack[0] : NULL;
}

int nq_scene_push(NqScene *s, NqNode *tree) {
    if (!s || !tree) return 0;
    if (s->depth >= NQ_SCENE_STACK_MAX) {
        NQ_LOG_WARN("nq_scene_push: stack full (%d), refusing push",
                    NQ_SCENE_STACK_MAX);
        return 0;
    }
    s->stack[s->depth++] = tree;
    return 1;
}

NqNode *nq_scene_pop(NqScene *s) {
    if (!s) return NULL;
    if (s->depth <= 1) return NULL;  /* can't pop below the implicit root */
    return s->stack[--s->depth];
}

NqNode *nq_scene_top(NqScene *s) {
    if (!s || s->depth == 0) return NULL;
    return s->stack[s->depth - 1];
}

size_t nq_scene_depth(NqScene *s) {
    return s ? s->depth : 0;
}

void nq_scene_update(NqScene *s, float dt) {
    if (!s || s->depth == 0) return;
    NqNode *top = s->stack[s->depth - 1];
    nq_node_update(top, dt);
    /* Also tick the implicit root so globally-attached systems can run
     * regardless of which top scene is active. The root's update is
     * suppressed (or runs in parallel) so user code in `top` is not
     * double-invoked for shared systems that hang off the root. */
    if (s->depth >= 1 && top != s->stack[0]) {
        nq_node_update(s->stack[0], dt);
    }
}

void nq_scene_draw(NqScene *s) {
    if (!s || s->depth == 0) return;
    /* Draw implicit root first (back of stack), then up to top in order.
     * This matches the convention: backgrounds under active UI. */
    for (size_t i = 0; i < s->depth; i++) {
        nq_node_draw(s->stack[i]);
    }
}
