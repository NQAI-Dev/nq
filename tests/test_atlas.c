#include <nq/atlas.h>
#include "test_main.c"

static void test_create_destroy(void) {
    NqAtlas *a = nq_atlas_create(512, 512, 0);
    NQ_ASSERT(a != NULL);
    NQ_ASSERT_EQ(nq_atlas_width(a), 512);
    NQ_ASSERT_EQ(nq_atlas_height(a), 512);
    nq_atlas_destroy(a);
}

static void test_add_find_roundtrip(void) {
    NqAtlas *a = nq_atlas_create(256, 256, 0);
    int idx = nq_atlas_add_region(a, "idle", nq_rect(0, 0, 32, 32));
    NQ_ASSERT(idx >= 0);
    NqRect r = nq_atlas_find(a, "idle");
    NQ_ASSERT_EQ(r.x, 0);
    NQ_ASSERT_EQ(r.y, 0);
    NQ_ASSERT_EQ(r.w, 32);
    NQ_ASSERT_EQ(r.h, 32);
    nq_atlas_destroy(a);
}

static void test_find_missing_returns_empty(void) {
    NqAtlas *a = nq_atlas_create(64, 64, 0);
    NqRect r = nq_atlas_find(a, "ghost");
    NQ_ASSERT(r.w <= 0 || r.h <= 0);  /* empty-rect convention */
    nq_atlas_destroy(a);
}

static void test_duplicate_name_rejected(void) {
    NqAtlas *a = nq_atlas_create(128, 128, 0);
    int i1 = nq_atlas_add_region(a, "frame", nq_rect(0, 0, 16, 16));
    NQ_ASSERT(i1 >= 0);
    int i2 = nq_atlas_add_region(a, "frame", nq_rect(16, 0, 16, 16));
    NQ_ASSERT_EQ(i2, -1);
    nq_atlas_destroy(a);
}

static void test_remove_region(void) {
    NqAtlas *a = nq_atlas_create(128, 128, 0);
    nq_atlas_add_region(a, "a", nq_rect(0, 0, 8, 8));
    nq_atlas_add_region(a, "b", nq_rect(8, 0, 8, 8));
    nq_atlas_add_region(a, "c", nq_rect(16, 0, 8, 8));
    NQ_ASSERT_EQ(nq_atlas_remove_region(a, "b"), 1);
    /* "a" and "c" still findable */
    NqRect ra = nq_atlas_find(a, "a");
    NqRect rc = nq_atlas_find(a, "c");
    NQ_ASSERT_EQ(ra.x, 0);
    NQ_ASSERT_EQ(rc.x, 16);
    /* "b" gone */
    NqRect rb = nq_atlas_find(a, "b");
    NQ_ASSERT(rb.w <= 0 && rb.h <= 0);
    /* removing again returns 0 */
    NQ_ASSERT_EQ(nq_atlas_remove_region(a, "b"), 0);
    nq_atlas_destroy(a);
}

static void test_remove_then_re_add(void) {
    NqAtlas *a = nq_atlas_create(64, 64, 0);
    int i1 = nq_atlas_add_region(a, "x", nq_rect(0, 0, 4, 4));
    NQ_ASSERT(i1 >= 0);
    nq_atlas_remove_region(a, "x");
    int i2 = nq_atlas_add_region(a, "x", nq_rect(8, 8, 4, 4));
    NQ_ASSERT(i2 >= 0);
    /* re-added region should be the new rect */
    NqRect r = nq_atlas_find(a, "x");
    NQ_ASSERT_EQ(r.x, 8);
    NQ_ASSERT_EQ(r.y, 8);
    nq_atlas_destroy(a);
}

static void test_iter_visits_only_live(void) {
    NqAtlas *a = nq_atlas_create(64, 64, 0);
    nq_atlas_add_region(a, "live1", nq_rect(0, 0, 4, 4));
    nq_atlas_add_region(a, "dead",  nq_rect(0, 0, 4, 4));
    nq_atlas_add_region(a, "live2", nq_rect(0, 0, 4, 4));
    nq_atlas_remove_region(a, "dead");
    NqAtlasIter *it = nq_atlas_iter_begin(a);
    int seen_live1 = 0, seen_live2 = 0, seen_dead = 0;
    const char *name = NULL;
    NqRect r;
    while (nq_atlas_iter_next(it, &name, &r)) {
        if (strcmp(name, "live1") == 0) seen_live1 = 1;
        else if (strcmp(name, "live2") == 0) seen_live2 = 1;
        else if (strcmp(name, "dead") == 0) seen_dead = 1;
    }
    nq_atlas_iter_end(it);
    NQ_ASSERT_EQ(seen_live1, 1);
    NQ_ASSERT_EQ(seen_live2, 1);
    NQ_ASSERT_EQ(seen_dead, 0);
    nq_atlas_destroy(a);
}

static void test_grow_beyond_initial_capacity(void) {
    /* Initial slot count is 16. Add 20 names to force realloc growth. */
    NqAtlas *a = nq_atlas_create(64, 64, 0);
    char name[16];
    for (int i = 0; i < 20; i++) {
        snprintf(name, sizeof(name), "r%d", i);
        int idx = nq_atlas_add_region(a, name, nq_rect(i, 0, 4, 4));
        NQ_ASSERT(idx >= 0);
    }
    /* All 20 retrievable */
    for (int i = 0; i < 20; i++) {
        snprintf(name, sizeof(name), "r%d", i);
        NqRect r = nq_atlas_find(a, name);
        NQ_ASSERT_EQ(r.x, i);
        NQ_ASSERT_EQ(r.w, 4);
    }
    nq_atlas_destroy(a);
}

NQ_TEST_REGISTER("atlas_create_destroy",          test_create_destroy);
NQ_TEST_REGISTER("atlas_add_find_roundtrip",       test_add_find_roundtrip);
NQ_TEST_REGISTER("atlas_find_missing_empty",       test_find_missing_returns_empty);
NQ_TEST_REGISTER("atlas_duplicate_rejected",       test_duplicate_name_rejected);
NQ_TEST_REGISTER("atlas_remove_region",            test_remove_region);
NQ_TEST_REGISTER("atlas_remove_then_readd",       test_remove_then_re_add);
NQ_TEST_REGISTER("atlas_iter_only_live",           test_iter_visits_only_live);
NQ_TEST_REGISTER("atlas_grows_beyond_initial",     test_grow_beyond_initial_capacity);

static void test_count_regions_tracks_live(void) {
    NqAtlas *a = nq_atlas_create(64, 64, 0);
    NQ_ASSERT_EQ(nq_atlas_count_regions(a), 0);
    nq_atlas_add_region(a, "a", nq_rect(0, 0, 8, 8));
    nq_atlas_add_region(a, "b", nq_rect(8, 0, 8, 8));
    nq_atlas_add_region(a, "c", nq_rect(16, 0, 8, 8));
    NQ_ASSERT_EQ(nq_atlas_count_regions(a), 3);
    /* Remove one — count drops by 1 */
    nq_atlas_remove_region(a, "b");
    NQ_ASSERT_EQ(nq_atlas_count_regions(a), 2);
    /* Add one — reuses the freed slot, count back to 3 */
    nq_atlas_add_region(a, "d", nq_rect(24, 0, 8, 8));
    NQ_ASSERT_EQ(nq_atlas_count_regions(a), 3);
    nq_atlas_destroy(a);
}

static void test_count_regions_null_safe(void) {
    NQ_ASSERT_EQ(nq_atlas_count_regions(NULL), 0);
}

NQ_TEST_REGISTER("atlas_count_regions_tracks_live", test_count_regions_tracks_live);
NQ_TEST_REGISTER("atlas_count_regions_null_safe",  test_count_regions_null_safe);
NQ_TEST_REGISTER("atlas_draw_null_safe",          test_atlas_draw_null_safe);
NQ_TEST_REGISTER("atlas_draw_unknown_region",     test_atlas_draw_unknown_region_null_texture);
