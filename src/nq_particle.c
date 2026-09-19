#include "nq/particle.h"
#include <stdlib.h>

struct NqParticleSystem {
    NqParticle *particles;
    uint32_t capacity;
    uint32_t count;
};

NqParticleSystem *nq_particle_system_create(uint32_t capacity) {
    if (capacity == 0) return NULL;
    NqParticleSystem *ps = (NqParticleSystem *)malloc(sizeof(NqParticleSystem));
    if (!ps) return NULL;
    ps->particles = (NqParticle *)malloc(sizeof(NqParticle) * capacity);
    if (!ps->particles) {
        free(ps);
        return NULL;
    }
    ps->capacity = capacity;
    ps->count = 0;
    return ps;
}

void nq_particle_system_destroy(NqParticleSystem *ps) {
    if (ps) {
        free(ps->particles);
        free(ps);
    }
}

void nq_particle_system_update(NqParticleSystem *ps, float delta_seconds) {
    if (!ps || delta_seconds <= 0.0f) return;
    
    for (uint32_t i = 0; i < ps->count; ) {
        ps->particles[i].life -= delta_seconds;
        if (ps->particles[i].life <= 0.0f) {
            /* Swap with last to remove */
            ps->count--;
            if (i < ps->count) {
                ps->particles[i] = ps->particles[ps->count];
            }
        } else {
            ps->particles[i].position.x += ps->particles[i].velocity.x * delta_seconds;
            ps->particles[i].position.y += ps->particles[i].velocity.y * delta_seconds;
            i++;
        }
    }
}

bool nq_particle_system_emit(NqParticleSystem *ps, const NqParticle *template_particle) {
    if (!ps || !template_particle || ps->count >= ps->capacity) return false;
    if (template_particle->life <= 0.0f) return false;
    
    NqParticle *p = &ps->particles[ps->count];
    *p = *template_particle;
    if (p->max_life <= 0.0f) {
        p->max_life = p->life;
    }
    ps->count++;
    return true;
}

uint32_t nq_particle_system_get_count(const NqParticleSystem *ps) {
    return ps ? ps->count : 0;
}

/* Linear interpolation helper for float */
static float lerp_f(float a, float b, float t) {
    return a + (b - a) * t;
}

/* Linear interpolation helper for color (assuming NqColor is struct {uint8_t r, g, b, a;}) */
static NqColor lerp_color(NqColor a, NqColor b, float t) {
    NqColor result;
    result.r = (uint8_t)(a.r + (b.r - a.r) * t);
    result.g = (uint8_t)(a.g + (b.g - a.g) * t);
    result.b = (uint8_t)(a.b + (b.b - a.b) * t);
    result.a = (uint8_t)(a.a + (b.a - a.a) * t);
    return result;
}

uint32_t nq_particle_system_get_render_data(const NqParticleSystem *ps,
                                            NqVec2f *positions,
                                            float *sizes,
                                            NqColor *colors) {
    if (!ps) return 0;
    
    for (uint32_t i = 0; i < ps->count; i++) {
        const NqParticle *p = &ps->particles[i];
        if (positions) positions[i] = p->position;
        
        float t = 1.0f - (p->life / p->max_life);
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        
        if (sizes) sizes[i] = lerp_f(p->start_size, p->end_size, t);
        if (colors) colors[i] = lerp_color(p->start_color, p->end_color, t);
    }
    
    return ps->count;
}
