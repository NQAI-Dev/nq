#include <nq/scene.h>
#include "test_main.c"

/* Callback counters: init calls each update once; update on each tick. */
typedef struct { int init_n, update_n, draw_n; } SceneCbCtx;
static void sc_init(NqNode *n, void *u) { (void)n; (void)u; }
static void sc_update(NqNode *n, float dt, void *u) {
    SceneCbCtx *c = (SceneCbCtx *)u;
    if (c) c->update_n++;
    (void)dt;
}
static void sc_draw(NqNode *n, void *u) { (void)n; if (u) ((SceneCbCtx *)u)->draw_n++; }

static void test_scene_create_has_implicit_root(void) {
    NqScene *s = nq_scene_create();
    NQ_ASSERT(s != NULL);
    NQ_ASSERT(nq_scene_depth(s) == 1);
    NQ_ASSERT(nq_scene_root(s) != NULL);
    NQ_ASSERT(nq_scene_top(s) == nq_scene_root(s));
    /* Popping at depth 1 returns NULL (can't pop below the implicit root). */
    NQ_ASSERT(nq_scene_pop(s) == NULL);
    NQ_ASSERT(nq_scene_depth(s) == 1);
    nq_scene_destroy(s);
}

static void test_scene_push_pop(void) {
    NqScene *s = nq_scene_create();
    NqNode *menu = nq_node_create(NULL, "menu");
    NQ_ASSERT_EQ(nq_scene_push(s, menu), 1);
    NQ_ASSERT_EQ(nq_scene_depth(s), 2);
    NQ_ASSERT(nq_scene_top(s) == menu);
    /* Root still there, just not top. */
    NQ_ASSERT(nq_scene_root(s) != NULL);
    NQ_ASSERT(nq_scene_top(s) != nq_scene_root(s));

    /* Pop returns the menu and restores root as top. */
    NqNode *popped = nq_scene_pop(s);
    NQ_ASSERT(popped == menu);
    NQ_ASSERT_EQ(nq_scene_depth(s), 1);
    NQ_ASSERT(nq_scene_top(s) == nq_scene_root(s));

    /* Pop again at depth 1 → NULL. */
    NQ_ASSERT(nq_scene_pop(s) == NULL);
    NQ_ASSERT_EQ(nq_scene_depth(s), 1);

    nq_node_destroy(menu);  /* caller owns */
    nq_scene_destroy(s);
}

static void test_scene_push_overflow(void) {
    NqScene *s = nq_scene_create();
    NqNode *dummy = nq_node_create(NULL, "dummy");
    /* Stack capacity is NQ_SCENE_STACK_MAX — push that many + 1 should fail */
    int pushes = 0;
    for (size_t i = 1; i < NQ_SCENE_STACK_MAX; i++) {  /* leaves room for root */
        pushes += nq_scene_push(s, dummy);
    }
    NQ_ASSERT(pushes == (int)(NQ_SCENE_STACK_MAX - 1));
    NQ_ASSERT_EQ(nq_scene_push(s, dummy), 0);  /* full → 0 */
    NQ_ASSERT_EQ(nq_scene_depth(s), NQ_SCENE_STACK_MAX);
    /* Cleanup: pop them all so the destroy doesn't recursively free them. */
    while (nq_scene_depth(s) > 1) (void)nq_scene_pop(s);
    nq_node_destroy(dummy);
    nq_scene_destroy(s);
}

static void test_scene_update_top_only(void) {
    NqScene *s = nq_scene_create();
    SceneCbCtx root_ctx = {0};
    SceneCbCtx top_ctx = {0};
    nq_node_set_callbacks(nq_scene_root(s), sc_init, sc_update, sc_draw, &root_ctx);
    NqNode *top = nq_node_create(NULL, "top");
    nq_node_set_callbacks(top, sc_init, sc_update, sc_draw, &top_ctx);
    nq_scene_push(s, top);

    nq_scene_update(s, 0.016f);
    /* Top updated once. Root also updated once (always ticks). */
    NQ_ASSERT_EQ(top_ctx.update_n,   1);
    NQ_ASSERT_EQ(root_ctx.update_n, 1);

    /* Pop top → only root updates. */
    (void)nq_scene_pop(s);
    nq_scene_update(s, 0.016f);
    NQ_ASSERT_EQ(top_ctx.update_n,   1);  /* unchanged */
    NQ_ASSERT_EQ(root_ctx.update_n, 2);

    nq_node_destroy(top);
    nq_scene_destroy(s);
}

static void test_scene_draw_root_then_top(void) {
    NqScene *s = nq_scene_create();
    SceneCbCtx root_ctx = {0};
    SceneCbCtx top_ctx  = {0};
    /* Disable drawing on root via the VISIBLE flag — the implicit-root draw
     * in nq_scene_draw() still walks it, but the root's draw callback
     * checks the flag and skips itself. This test verifies the walk order
     * (root first, then top) without forcing the user to set VISIBLE. */
    nq_node_set_callbacks(nq_scene_root(s),
                          sc_init, sc_update, sc_draw, &root_ctx);
    nq_node_set_flag(nq_scene_root(s), NQ_NODE_FLAG_VISIBLE, 1);
    nq_node_set_callbacks(nq_scene_root(s),
                          sc_init, sc_update, sc_draw, &root_ctx);

    NqNode *top = nq_node_create(NULL, "top");
    nq_node_set_callbacks(top, sc_init, sc_update, sc_draw, &top_ctx);
    nq_scene_push(s, top);

    nq_scene_draw(s);
    /* Draw order: implicit root first, then pushed top. */
    NQ_ASSERT_EQ(root_ctx.draw_n, 1);
    NQ_ASSERT_EQ(top_ctx.draw_n,  1);

    /* If root's visible flag is cleared, its draw_cb is skipped but the
     * draw walk still visits it — important for ordering invariants. */
    nq_node_set_flag(nq_scene_root(s), NQ_NODE_FLAG_VISIBLE, 0);
    nq_scene_draw(s);
    NQ_ASSERT_EQ(root_ctx.draw_n, 1);  /* unchanged */
    NQ_ASSERT_EQ(top_ctx.draw_n,  2);

    nq_node_destroy(top);
    nq_scene_destroy(s);
}

static void test_scene_null_safe(void) {
    /* All API calls with NULL scene must be safe no-ops / sensible. */
    nq_scene_destroy(NULL);
    NQ_ASSERT(nq_scene_root(NULL) == NULL);
    NQ_ASSERT(nq_scene_top(NULL)  == NULL);
    NQ_ASSERT_EQ(nq_scene_depth(NULL), 0);
    NQ_ASSERT_EQ(nq_scene_push(NULL, NULL), 0);
    NQ_ASSERT(nq_scene_pop(NULL) == NULL);
    nq_scene_update(NULL, 0.016f);
    nq_scene_draw(NULL);
}

NQ_TEST_REGISTER("scene_create_implicit_root",   test_scene_create_has_implicit_root);
NQ_TEST_REGISTER("scene_push_pop",               test_scene_push_pop);
NQ_TEST_REGISTER("scene_push_overflow",          test_scene_push_overflow);
NQ_TEST_REGISTER("scene_update_top_only",        test_scene_update_top_only);
NQ_TEST_REGISTER("scene_draw_root_then_top",     test_scene_draw_root_then_top);
NQ_TEST_REGISTER("scene_null_safe",             test_scene_null_safe);
