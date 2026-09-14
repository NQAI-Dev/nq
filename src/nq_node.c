#include "nq/node.h"
#include "nq/log.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct NqNode {
    char        name[NQ_NODE_NAME_MAX];
    NqNode     *parent;
    NqNode     *first_child;
    NqNode     *next_sibling;

    nq_node_init_fn   init_cb;
    nq_node_update_fn update_cb;
    nq_node_draw_fn   draw_cb;
    void             *user;

    float pos[2];      /* local position */
    float scale[2];    /* local scale */
    float rotation;    /* degrees */
    int   flags;       /* NqNodeFlags bitmask */
    int   initialized; /* 1 once init_cb has fired */
};

static void nq_node_walk_pre_order(NqNode *n,
                                   void (*visit)(NqNode *, void *),
                                   void *visit_user) {
    if (!n || !visit) return;
    visit(n, visit_user);
    for (NqNode *c = n->first_child; c; c = c->next_sibling) {
        nq_node_walk_pre_order(c, visit, visit_user);
    }
}

static void invoke_init_visit(NqNode *n, void *user) {
    (void)user;
    if (n->init_cb && !n->initialized) {
        n->init_cb(n, n->user);
        n->initialized = 1;
    }
}

static void invoke_update_visit(NqNode *n, void *user) {
    float dt = *(float *)user;
    if (n->update_cb && (n->flags & NQ_NODE_FLAG_UPDATING)) {
        n->update_cb(n, dt, n->user);
    }
}

static void invoke_draw_visit(NqNode *n, void *user) {
    (void)user;
    if (n->draw_cb && (n->flags & NQ_NODE_FLAG_VISIBLE)) {
        n->draw_cb(n, n->user);
    }
}

NqNode *nq_node_create(NqNode *parent, const char *name) {
    NqNode *n = calloc(1, sizeof(NqNode));
    if (!n) {
        NQ_LOG_ERROR("nq_node_create: out of memory");
        return NULL;
    }
    if (name) {
        strncpy(n->name, name, NQ_NODE_NAME_MAX - 1);
        n->name[NQ_NODE_NAME_MAX - 1] = '\0';
    } else {
        strcpy(n->name, "node");
    }
    n->flags = NQ_NODE_FLAG_ALIVE | NQ_NODE_FLAG_VISIBLE | NQ_NODE_FLAG_UPDATING;
    n->pos[0] = 0.0f;
    n->pos[1] = 0.0f;
    n->scale[0] = 1.0f;
    n->scale[1] = 1.0f;
    n->rotation = 0.0f;

    if (parent) {
        n->parent = parent;
        n->next_sibling = parent->first_child;
        parent->first_child = n;
    }
    return n;
}

void nq_node_destroy(NqNode *n) {
    if (!n || !(n->flags & NQ_NODE_FLAG_ALIVE)) return;
    /* Recursively destroy children first so each parent's first_child
     * invariant is maintained. */
    while (n->first_child) {
        nq_node_destroy(n->first_child);
    }
    /* Detach from parent. */
    if (n->parent) {
        if (n->parent->first_child == n) {
            n->parent->first_child = n->next_sibling;
        } else {
            for (NqNode *sib = n->parent->first_child; sib; sib = sib->next_sibling) {
                if (sib->next_sibling == n) {
                    sib->next_sibling = n->next_sibling;
                    break;
                }
            }
        }
    }
    n->flags &= ~NQ_NODE_FLAG_ALIVE;
    free(n);
}

void nq_node_attach(NqNode *child, NqNode *new_parent) {
    if (!child) return;
    /* Detach from current parent. */
    if (child->parent) {
        if (child->parent->first_child == child) {
            child->parent->first_child = child->next_sibling;
        } else {
            for (NqNode *sib = child->parent->first_child; sib; sib = sib->next_sibling) {
                if (sib->next_sibling == child) {
                    sib->next_sibling = child->next_sibling;
                    break;
                }
            }
        }
        child->next_sibling = NULL;
    }
    child->parent = new_parent;
    if (new_parent) {
        child->next_sibling = new_parent->first_child;
        new_parent->first_child = child;
    }
}

NqNode *nq_node_parent(const NqNode *n)      { return n ? n->parent : NULL; }
NqNode *nq_node_first_child(const NqNode *n) { return n ? n->first_child : NULL; }
NqNode *nq_node_next_sibling(const NqNode *n) { return n ? n->next_sibling : NULL; }

void nq_node_set_flag(NqNode *n, NqNodeFlags flag, int on) {
    if (!n) return;
    if (on) n->flags |= flag;
    else    n->flags &= ~flag;
}

int nq_node_get_flag(NqNode *n, NqNodeFlags flag) {
    return n ? !!(n->flags & flag) : 0;
}

void nq_node_set_position(NqNode *n, float x, float y) {
    if (!n) return;
    n->pos[0] = x;
    n->pos[1] = y;
}

void nq_node_set_rotation(NqNode *n, float degrees) {
    if (!n) return;
    n->rotation = degrees;
}

void nq_node_set_scale(NqNode *n, float sx, float sy) {
    if (!n) return;
    if (sx == 0.0f) sx = 1e-6f;
    if (sy == 0.0f) sy = 1e-6f;
    n->scale[0] = sx;
    n->scale[1] = sy;
}

void nq_node_get_position(const NqNode *n, float *out_x, float *out_y) {
    if (!n) return;
    if (out_x) *out_x = n->pos[0];
    if (out_y) *out_y = n->pos[1];
}

float nq_node_get_rotation(const NqNode *n) {
    return n ? n->rotation : 0.0f;
}

void nq_node_get_scale(const NqNode *n, float *out_sx, float *out_sy) {
    if (!n) return;
    if (out_sx) *out_sx = n->scale[0];
    if (out_sy) *out_sy = n->scale[1];
}

void nq_node_set_callbacks(NqNode *n,
                           nq_node_init_fn init,
                           nq_node_update_fn update,
                           nq_node_draw_fn draw,
                           void *user) {
    if (!n) return;
    n->init_cb   = init;
    n->update_cb = update;
    n->draw_cb   = draw;
    n->user      = user;
}

void nq_node_update(NqNode *root, float dt) {
    if (!root) return;
    /* First pass: trigger init once per node. */
    nq_node_walk_pre_order(root, invoke_init_visit, NULL);
    /* Second pass: update per frame. */
    nq_node_walk_pre_order(root, invoke_update_visit, &dt);
}

void nq_node_draw(NqNode *root) {
    if (!root) return;
    nq_node_walk_pre_order(root, invoke_draw_visit, NULL);
}
