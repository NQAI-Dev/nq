#include "nq/particle.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

#define FLOAT_EQ(a, b) (fabs((a) - (b)) < 0.0001f)

static void test_create_destroy() {
    NqParticleSystem *ps = nq_particle_system_create(0);
    assert(ps == NULL);

    ps = nq_particle_system_create(100);
    assert(ps != NULL);
    assert(nq_particle_system_get_count(ps) == 0);
    nq_particle_system_destroy(ps);
}

static void test_emit_and_update() {
    NqParticleSystem *ps = nq_particle_system_create(2);
    assert(ps != NULL);

    NqParticle p = {
        .position = {0.0f, 0.0f},
        .velocity = {10.0f, 0.0f},
        .life = 1.0f,
        .max_life = 1.0f,
        .start_color = {255, 0, 0, 255},
        .end_color = {0, 255, 0, 255},
        .start_size = 1.0f,
        .end_size = 2.0f
    };

    bool emitted = nq_particle_system_emit(ps, &p);
    assert(emitted);
    assert(nq_particle_system_get_count(ps) == 1);

    emitted = nq_particle_system_emit(ps, &p);
    assert(emitted);
    assert(nq_particle_system_get_count(ps) == 2);

    /* Test capacity limit */
    emitted = nq_particle_system_emit(ps, &p);
    assert(!emitted);
    assert(nq_particle_system_get_count(ps) == 2);

    /* Update with 0.5s (half life) */
    nq_particle_system_update(ps, 0.5f);
    assert(nq_particle_system_get_count(ps) == 2);

    NqVec2f pos[2];
    float sizes[2];
    NqColor colors[2];
    uint32_t count = nq_particle_system_get_render_data(ps, pos, sizes, colors);
    assert(count == 2);
    
    assert(FLOAT_EQ(pos[0].x, 5.0f));
    assert(FLOAT_EQ(pos[0].y, 0.0f));
    assert(FLOAT_EQ(sizes[0], 1.5f));
    assert(colors[0].r == 127); /* half way between 255 and 0 */

    /* Update past life */
    nq_particle_system_update(ps, 0.6f);
    assert(nq_particle_system_get_count(ps) == 0);

    nq_particle_system_destroy(ps);
}

int main() {
    test_create_destroy();
    test_emit_and_update();
    printf("test_particle passed\n");
    return 0;
}
