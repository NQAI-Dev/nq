#include <nq/node.h>
#include "test_main.c"

/* Test callbacks — count calls, capture this, capture dt. */
typedef struct {
    int init_calls;
    int update_calls;
    int draw_calls;
    NqNode *last_self;
    float last_dt;
    int saw_root;
    int saw_child;
} NodeCbCtx;

static void test_node_init(NqNode *n, void *user) {
    NodeCbCtx *ctx = (NodeCbCtx *)user;
    if (!ctx) return;
    ctx->init_calls++;
    ctx->last_self = n;
}

static void test_node_update(NqNode *n, float dt, void *user) {
    NodeCbCtx *ctx = (NodeCbCtx *)user;
    if (!ctx) return;
    ctx->update_calls++;
    ctx->last_dt = dt;
    if (n == ctx->last_self) ctx->saw_child = 1;
    if (ctx->init_calls == 1 && ctx->update_calls == 1) ctx->saw_root = 1;
}

static void test_node_draw(NqNode *n, void *user) {
    NodeCbCtx *ctx = (NodeCbCtx *)user;
    if (!ctx) return;
    ctx->draw_calls++;
}

static void test_create_root_and_child(void) {
    NqNode *root = nq_node_create(NULL, "root");
    NQ_ASSERT(root != NULL);
    NQ_ASSERT(nq_node_parent(root) == NULL);
    NQ_ASSERT(nq_node_first_child(root) == NULL);

    NqNode *kid = nq_node_create(root, "kid");
    NQ_ASSERT(kid != NULL);
    NQ_ASSERT(nq_node_parent(kid) == root);
    NQ_ASSERT(nq_node_first_child(root) == kid);
    NQ_ASSERT(nq_node_next_sibling(kid) == NULL);

    /* Sibling order: most recent child is at the head of the list. */
    NqNode *kid2 = nq_node_create(root, "kid2");
    NQ_ASSERT(nq_node_first_child(root) == kid2);  /* newest head */
    NQ_ASSERT(nq_node_next_sibling(kid2) == kid);   /* kid2 -> kid */

    nq_node_destroy(root);
}

static void test_destroy_subtree(void) {
    NqNode *root = nq_node_create(NULL, "root");
    NqNode *kid = nq_node_create(root, "kid");
    NqNode *grand = nq_node_create(kid, "grand");
    (void)grand;
    nq_node_destroy(kid);
    /* root remains intact */
    NQ_ASSERT(nq_node_first_child(root) == NULL);
    nq_node_destroy(root);
}

static void test_flags_default_and_set(void) {
    NqNode *n = nq_node_create(NULL, "n");
    /* Default: ALIVE | VISIBLE | UPDATING */
    NQ_ASSERT(nq_node_get_flag(n, NQ_NODE_FLAG_ALIVE));
    NQ_ASSERT(nq_node_get_flag(n, NQ_NODE_FLAG_VISIBLE));
    NQ_ASSERT(nq_node_get_flag(n, NQ_NODE_FLAG_UPDATING));

    nq_node_set_flag(n, NQ_NODE_FLAG_VISIBLE, 0);
    NQ_ASSERT_EQ(nq_node_get_flag(n, NQ_NODE_FLAG_VISIBLE), 0);
    NQ_ASSERT_EQ(nq_node_get_flag(n, NQ_NODE_FLAG_ALIVE), 1);

    nq_node_set_flag(n, NQ_NODE_FLAG_VISIBLE, 1);
    NQ_ASSERT_EQ(nq_node_get_flag(n, NQ_NODE_FLAG_VISIBLE), 1);

    nq_node_destroy(n);
}

static void test_transform_get_set(void) {
    NqNode *n = nq_node_create(NULL, "n");
    nq_node_set_position(n, 10.0f, 20.0f);
    nq_node_set_rotation(n, 45.0f);
    nq_node_set_scale(n, 2.0f, 3.0f);

    float x, y, sx, sy;
    nq_node_get_position(n, &x, &y);
    NQ_FE(x, 10.0f); NQ_FE(y, 20.0f);
    NQ_FE(nq_node_get_rotation(n), 45.0f);
    nq_node_get_scale(n, &sx, &sy);
    NQ_FE(sx, 2.0f); NQ_FE(sy, 3.0f);

    nq_node_destroy(n);
}

static void test_update_walks_subtree_pre_order(void) {
    NodeCbCtx ctx = {0};
    NqNode *root = nq_node_create(NULL, "r");
    nq_node_set_callbacks(root, test_node_init, test_node_update, test_node_draw, &ctx);
    NqNode *kid = nq_node_create(root, "k");
    nq_node_set_callbacks(kid, test_node_init, test_node_update, test_node_draw, &ctx);

    nq_node_update(root, 0.016f);
    NQ_ASSERT_EQ(ctx.init_calls,   2);  /* root + kid */
    NQ_ASSERT_EQ(ctx.update_calls, 2);
    NQ_ASSERT_EQ(ctx.draw_calls,   0);

    nq_node_update(root, 0.016f);
    /* init fires once per node — second update should not re-init */
    NQ_ASSERT_EQ(ctx.init_calls,   2);
    NQ_ASSERT_EQ(ctx.update_calls, 4);  /* each tick = +2 (root + kid) */

    nq_node_destroy(root);
}

static void test_update_dt_is_passed(void) {
    NodeCbCtx ctx = {0};
    NqNode *root = nq_node_create(NULL, "r");
    nq_node_set_callbacks(root, test_node_init, test_node_update, test_node_draw, &ctx);
    nq_node_update(root, 0.033f);
    NQ_FE(ctx.last_dt, 0.033f);
    nq_node_destroy(root);
}

static void test_draw_skipped_when_invisible(void) {
    NodeCbCtx ctx = {0};
    NqNode *root = nq_node_create(NULL, "r");
    nq_node_set_callbacks(root, test_node_init, test_node_update, test_node_draw, &ctx);
    nq_node_set_flag(root, NQ_NODE_FLAG_VISIBLE, 0);
    nq_node_draw(root);
    NQ_ASSERT_EQ(ctx.draw_calls, 0);
    nq_node_set_flag(root, NQ_NODE_FLAG_VISIBLE, 1);
    nq_node_draw(root);
    NQ_ASSERT_EQ(ctx.draw_calls, 1);
    nq_node_destroy(root);
}

static void test_attach_reparent(void) {
    NqNode *root1 = nq_node_create(NULL, "r1");
    NqNode *root2 = nq_node_create(NULL, "r2");
    NqNode *kid   = nq_node_create(root1, "k");

    NQ_ASSERT(nq_node_parent(kid) == root1);
    nq_node_attach(kid, root2);
    NQ_ASSERT(nq_node_parent(kid) == root2);
    NQ_ASSERT(nq_node_first_child(root1) == NULL);
    NQ_ASSERT(nq_node_first_child(root2) == kid);

    /* detach to root via NULL */
    nq_node_attach(kid, NULL);
    NQ_ASSERT(nq_node_parent(kid) == NULL);

    nq_node_destroy(kid);
    nq_node_destroy(root1);
    nq_node_destroy(root2);
}

static void test_null_safety(void) {
    nq_node_destroy(NULL);
    NQ_ASSERT_EQ(nq_node_get_flag(NULL, NQ_NODE_FLAG_ALIVE), 0);
    nq_node_set_position(NULL, 1, 2);  /* no-op */
    nq_node_set_rotation(NULL, 90);
    nq_node_set_scale(NULL, 2, 2);
    nq_node_set_callbacks(NULL, NULL, NULL, NULL, NULL);
    nq_node_update(NULL, 0.016f);
    nq_node_draw(NULL);
    NQ_ASSERT(nq_node_parent(NULL) == NULL);
    NQ_ASSERT(nq_node_first_child(NULL) == NULL);
    NQ_ASSERT(nq_node_next_sibling(NULL) == NULL);
    float x = -1, y = -1, sx = -1, sy = -1;
    nq_node_get_position(NULL, &x, &y);
    NQ_ASSERT(x == -1 && y == -1);
    nq_node_get_scale(NULL, &sx, &sy);
    NQ_ASSERT(sx == -1 && sy == -1);
}

NQ_TEST_REGISTER("node_create_root_and_child",      test_create_root_and_child);
NQ_TEST_REGISTER("node_destroy_subtree",            test_destroy_subtree);
NQ_TEST_REGISTER("node_flags_default_and_set",      test_flags_default_and_set);
NQ_TEST_REGISTER("node_transform_get_set",          test_transform_get_set);
NQ_TEST_REGISTER("node_update_walks_subtree",      test_update_walks_subtree_pre_order);
NQ_TEST_REGISTER("node_update_dt_passed",           test_update_dt_is_passed);
NQ_TEST_REGISTER("node_draw_skipped_when_inv",     test_draw_skipped_when_invisible);
NQ_TEST_REGISTER("node_attach_reparent",            test_attach_reparent);
NQ_TEST_REGISTER("node_null_safety",                test_null_safety);
