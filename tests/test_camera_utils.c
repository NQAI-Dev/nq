#include "nq/camera_utils.h"
#include <assert.h>
#include <math.h>
#include "test_main.c"

#define ASSERT_EQ(a, b) do { if ((a) != (b)) { return; } } while(0)
#define ASSERT_TRUE(a) do { if (!(a)) { return; } } while(0)

static void test_get_viewport_rect(void) {
    NqCamera2D cam;
    NqVec2f center = {400.0f, 300.0f};
    NqRect vp_screen = {0, 0, 800, 600};
    nq_camera2d_init(&cam, center, vp_screen);
    cam.zoom = 1.0f;

    NqRect vp = nq_camera2d_get_viewport_rect(&cam);
    ASSERT_EQ(vp.x, 0);
    ASSERT_EQ(vp.y, 0);
    ASSERT_EQ(vp.w, 800);
    ASSERT_EQ(vp.h, 600);

    cam.zoom = 2.0f;
    vp = nq_camera2d_get_viewport_rect(&cam);
    ASSERT_EQ(vp.x, 200);
    ASSERT_EQ(vp.y, 150);
    ASSERT_EQ(vp.w, 400);
    ASSERT_EQ(vp.h, 300);
}

static void test_clamp_to_bounds(void) {
    NqCamera2D cam;
    NqVec2f center = {400.0f, 300.0f};
    NqRect vp_screen = {0, 0, 800, 600};
    nq_camera2d_init(&cam, center, vp_screen);
    cam.zoom = 1.0f;
    
    NqRect bounds = {0, 0, 2000, 2000};
    
    // Try to move out of bounds top-left
    cam.center.x = 0.0f;
    cam.center.y = 0.0f;
    nq_camera2d_clamp_to_bounds(&cam, bounds);
    ASSERT_TRUE(fabsf(cam.center.x - 400.0f) < 0.001f);
    ASSERT_TRUE(fabsf(cam.center.y - 300.0f) < 0.001f);
    
    // Try to move out of bounds bottom-right
    cam.center.x = 2500.0f;
    cam.center.y = 2500.0f;
    nq_camera2d_clamp_to_bounds(&cam, bounds);
    ASSERT_TRUE(fabsf(cam.center.x - 1600.0f) < 0.001f);
    ASSERT_TRUE(fabsf(cam.center.y - 1700.0f) < 0.001f);
    
    // Test view larger than bounds (centers view)
    cam.zoom = 0.1f; // view is 8000x6000
    cam.center.x = 0.0f;
    cam.center.y = 0.0f;
    nq_camera2d_clamp_to_bounds(&cam, bounds);
    ASSERT_TRUE(fabsf(cam.center.x - 1000.0f) < 0.001f);
    ASSERT_TRUE(fabsf(cam.center.y - 1000.0f) < 0.001f);
}

static void test_drag_pan(void) {
    NqCamera2D cam;
    NqVec2f center = {400.0f, 300.0f};
    NqRect vp_screen = {0, 0, 800, 600};
    nq_camera2d_init(&cam, center, vp_screen);
    cam.zoom = 1.0f;

    NqInput input;
    nq_input_init(&input);
    nq_input_set_mouse_pos(&input, 50, -20);
    

    nq_camera2d_drag_pan(&cam, &input, true);
    
    ASSERT_TRUE(fabsf(cam.center.x - 350.0f) < 0.001f);
    ASSERT_TRUE(fabsf(cam.center.y - 320.0f) < 0.001f);
}

/* Registration */
#ifndef NQ_TEST_MAX
#define NQ_TEST_MAX 256
typedef void (*nq_test_fn)(void);
typedef struct { const char *name; nq_test_fn fn; } nq_test_entry;
extern nq_test_entry nq_test_table[NQ_TEST_MAX];
extern int nq_test_count;
#define NQ_TEST_REGISTER(test_name, test_fn)                                  \
    __attribute__((constructor)) static void nq_test_reg_##test_fn(void) {   \
        if (nq_test_count < NQ_TEST_MAX) {                                 \
            nq_test_table[nq_test_count].name = (test_name);                 \
            nq_test_table[nq_test_count].fn   = (test_fn);                   \
            nq_test_count++;                                                \
        }                                                                   \
    }
#endif

NQ_TEST_REGISTER("camera_utils_get_viewport", test_get_viewport_rect)
NQ_TEST_REGISTER("camera_utils_clamp_to_bounds", test_clamp_to_bounds)
NQ_TEST_REGISTER("camera_utils_drag_pan", test_drag_pan)
