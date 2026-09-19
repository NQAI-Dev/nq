#include "nq/raycast.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static int near(float actual, float expected) {
    return fabsf(actual - expected) < 1e-4f;
}

int main(void) {
    const NqVec2f center = {10.0f, 10.0f};
    nq_ray ray = {{0.0f, 10.0f}, {1.0f, 0.0f}};
    nq_raycast_hit hit = nq_raycast_circle(ray, center, 3.0f, 100.0f);

    assert(hit.hit);
    assert(near(hit.distance, 7.0f));
    assert(near(hit.point.x, 7.0f));
    assert(near(hit.point.y, 10.0f));
    assert(near(hit.normal.x, -1.0f));
    assert(near(hit.normal.y, 0.0f));

    ray.origin.y = 14.0f;
    assert(!nq_raycast_circle(ray, center, 3.0f, 100.0f).hit);

    ray.origin.y = 13.0f;
    hit = nq_raycast_circle(ray, center, 3.0f, 100.0f);
    assert(hit.hit);
    assert(near(hit.distance, 10.0f));
    assert(near(hit.normal.x, 0.0f));
    assert(near(hit.normal.y, 1.0f));

    ray.origin.x = 10.0f;
    ray.origin.y = 10.0f;
    hit = nq_raycast_circle(ray, center, 3.0f, 100.0f);
    assert(hit.hit);
    assert(near(hit.distance, 0.0f));
    assert(near(hit.point.x, 10.0f));
    assert(near(hit.normal.x, -1.0f));

    ray.origin.x = 0.0f;
    assert(!nq_raycast_circle(ray, center, 3.0f, 6.99f).hit);
    assert(!nq_raycast_circle(ray, center, -1.0f, 100.0f).hit);
    assert(!nq_raycast_circle(ray, center, 3.0f, -1.0f).hit);

    ray.origin.x = 10.0f;
    ray.origin.y = 10.0f;
    ray.dir.x = 0.0f;
    ray.dir.y = 0.0f;
    assert(nq_raycast_circle(ray, center, 0.0f, 0.0f).hit);
    ray.origin.x = 14.0f;
    assert(!nq_raycast_circle(ray, center, 3.0f, 100.0f).hit);

    printf("All test_raycast_circle passed.\n");
    return 0;
}
