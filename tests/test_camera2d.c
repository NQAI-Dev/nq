#include <math.h>

#include <nq/camera2d.h>
#include "test_main.c"

#define NQ_ASSERT_NEAR(a, b) \
    NQ_ASSERT(fabsf((float)(a) - (float)(b)) < 1e-5f)

static void test_camera2d_init_and_center_mapping(void) {
    NqCamera2D camera;
    nq_camera2d_init(&camera, nq_vec2f(10.0f, -5.0f), nq_rect(20, 30, 800, 600));

    NQ_ASSERT_NEAR(camera.zoom, 1.0f);
    NqVec2f screen = nq_camera2d_world_to_screen(&camera, camera.center);
    NQ_ASSERT_NEAR(screen.x, 420.0f);
    NQ_ASSERT_NEAR(screen.y, 330.0f);
}

static void test_camera2d_round_trip_with_zoom(void) {
    NqCamera2D camera;
    nq_camera2d_init(&camera, nq_vec2f(-14.0f, 23.5f), nq_rect(11, 7, 641, 479));
    NQ_ASSERT(nq_camera2d_set_zoom(&camera, 2.5f));

    NqVec2f world = nq_vec2f(31.25f, -8.0f);
    NqVec2f restored = nq_camera2d_screen_to_world(
        &camera, nq_camera2d_world_to_screen(&camera, world));
    NQ_ASSERT_NEAR(restored.x, world.x);
    NQ_ASSERT_NEAR(restored.y, world.y);
}

static void test_camera2d_pan_moves_world_opposite(void) {
    NqCamera2D camera;
    nq_camera2d_init(&camera, nq_vec2f(0.0f, 0.0f), nq_rect(0, 0, 100, 100));
    NqVec2f before = nq_camera2d_world_to_screen(&camera, nq_vec2f(20.0f, 20.0f));

    nq_camera2d_pan(&camera, nq_vec2f(5.0f, -3.0f));
    NqVec2f after = nq_camera2d_world_to_screen(&camera, nq_vec2f(20.0f, 20.0f));
    NQ_ASSERT_NEAR(after.x, before.x - 5.0f);
    NQ_ASSERT_NEAR(after.y, before.y + 3.0f);
}

static void test_camera2d_rejects_invalid_zoom(void) {
    NqCamera2D camera;
    nq_camera2d_init(&camera, nq_vec2f(1.0f, 2.0f), nq_rect(0, 0, 100, 100));

    NQ_ASSERT(!nq_camera2d_set_zoom(&camera, 0.0f));
    NQ_ASSERT(!nq_camera2d_set_zoom(&camera, -1.0f));
    NQ_ASSERT(!nq_camera2d_set_zoom(&camera, NAN));
    NQ_ASSERT_NEAR(camera.zoom, 1.0f);
}

static void test_camera2d_zoom_at_preserves_anchor(void) {
    NqCamera2D camera;
    nq_camera2d_init(&camera, nq_vec2f(10.0f, 20.0f), nq_rect(50, 25, 400, 300));
    NqVec2f anchor = nq_vec2f(175.0f, 100.0f);
    NqVec2f world_before = nq_camera2d_screen_to_world(&camera, anchor);

    NQ_ASSERT(nq_camera2d_zoom_at(&camera, 4.0f, anchor));
    NqVec2f world_after = nq_camera2d_screen_to_world(&camera, anchor);
    NQ_ASSERT_NEAR(world_after.x, world_before.x);
    NQ_ASSERT_NEAR(world_after.y, world_before.y);
}

static void test_camera2d_null_is_safe(void) {
    nq_camera2d_init(NULL, nq_vec2f(0.0f, 0.0f), nq_rect(0, 0, 1, 1));
    nq_camera2d_pan(NULL, nq_vec2f(1.0f, 1.0f));
    NQ_ASSERT(!nq_camera2d_set_zoom(NULL, 2.0f));
    NQ_ASSERT(!nq_camera2d_zoom_at(NULL, 2.0f, nq_vec2f(0.0f, 0.0f)));
    NQ_ASSERT(nq_vec2f_eq(nq_camera2d_world_to_screen(NULL, nq_vec2f(1.0f, 2.0f)),
                          nq_vec2f(0.0f, 0.0f)));
}

NQ_TEST_REGISTER("camera2d_init_and_center_mapping", test_camera2d_init_and_center_mapping)
NQ_TEST_REGISTER("camera2d_round_trip_with_zoom", test_camera2d_round_trip_with_zoom)
NQ_TEST_REGISTER("camera2d_pan_moves_world_opposite", test_camera2d_pan_moves_world_opposite)
NQ_TEST_REGISTER("camera2d_rejects_invalid_zoom", test_camera2d_rejects_invalid_zoom)
NQ_TEST_REGISTER("camera2d_zoom_at_preserves_anchor", test_camera2d_zoom_at_preserves_anchor)
NQ_TEST_REGISTER("camera2d_null_is_safe", test_camera2d_null_is_safe)
