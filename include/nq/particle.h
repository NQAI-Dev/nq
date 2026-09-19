#ifndef NQ_PARTICLE_H
#define NQ_PARTICLE_H

#include <stdbool.h>
#include <stdint.h>
#include "nq/vec.h"
#include "nq/color.h"

typedef struct {
    NqVec2f position;
    NqVec2f velocity;
    float life;
    float max_life;
    NqColor start_color;
    NqColor end_color;
    float start_size;
    float end_size;
} NqParticle;

typedef struct NqParticleSystem NqParticleSystem;

/* Creates a particle system with a fixed maximum capacity. */
NqParticleSystem *nq_particle_system_create(uint32_t capacity);

/* Destroys the particle system and frees memory. */
void nq_particle_system_destroy(NqParticleSystem *ps);

/* Updates all alive particles using delta_seconds. */
void nq_particle_system_update(NqParticleSystem *ps, float delta_seconds);

/* Spawns a new particle. Returns true if spawned, false if at capacity. */
bool nq_particle_system_emit(NqParticleSystem *ps, const NqParticle *template_particle);

/* Returns the number of currently alive particles. */
uint32_t nq_particle_system_get_count(const NqParticleSystem *ps);

/* Fills the provided arrays (must be sized to capacity) with data for rendering.
 * Returns the number of active particles written. */
uint32_t nq_particle_system_get_render_data(const NqParticleSystem *ps,
                                            NqVec2f *positions,
                                            float *sizes,
                                            NqColor *colors);

#endif
