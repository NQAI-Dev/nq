#include "nq/raycast.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

int main(void) {
    NqRect box = {10, 10, 10, 10}; // box at (10,10) to (20,20)
    
    // Test hit from left
    nq_ray ray1; ray1.origin.x=0.0f; ray1.origin.y=15.0f; ray1.dir.x=1.0f; ray1.dir.y=0.0f;
    nq_raycast_hit hit1 = nq_raycast_aabb(ray1, box, 100.0f);
    assert(hit1.hit);
    assert(fabsf(hit1.distance - 10.0f) < 1e-4f);
    assert(fabsf(hit1.point.x - 10.0f) < 1e-4f);
    assert(fabsf(hit1.point.y - 15.0f) < 1e-4f);
    assert(fabsf(hit1.normal.x - -1.0f) < 1e-4f);
    assert(fabsf(hit1.normal.y - 0.0f) < 1e-4f);
    
    // Test miss
    nq_ray ray2; ray2.origin.x=0.0f; ray2.origin.y=5.0f; ray2.dir.x=1.0f; ray2.dir.y=0.0f;
    nq_raycast_hit hit2 = nq_raycast_aabb(ray2, box, 100.0f);
    assert(!hit2.hit);
    
    // Test inside
    nq_ray ray3; ray3.origin.x=15.0f; ray3.origin.y=15.0f; ray3.dir.x=1.0f; ray3.dir.y=0.0f;
    nq_raycast_hit hit3 = nq_raycast_aabb(ray3, box, 100.0f);
    assert(hit3.hit);
    assert(fabsf(hit3.distance) < 1e-4f);
    
    // Test distance limit
    nq_ray ray4; ray4.origin.x=0.0f; ray4.origin.y=15.0f; ray4.dir.x=1.0f; ray4.dir.y=0.0f;
    nq_raycast_hit hit4 = nq_raycast_aabb(ray4, box, 5.0f);
    assert(!hit4.hit);

    printf("All test_raycast passed.\n");
    return 0;
}
