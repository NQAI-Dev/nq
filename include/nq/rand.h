/*
 * nq — deterministic pseudo-random number generator (PCG32).
 *
 * Implements the PCG32 algorithm (M.E. O'Neill, 2014). Produces 32-bit
 * output with a period of 2^64. Two independent streams for the same
 * seed are obtained by passing different `seq` values.
 *
 * All functions are pure / stateless given the NqRand struct; no globals.
 * Thread-safe when each thread holds its own NqRand.
 *
 * References: https://www.pcg-random.org/
 */
#ifndef NQ_RAND_H
#define NQ_RAND_H

#include <stdint.h>

/* State for one independent RNG stream. Initialise with nq_rand_seed(). */
typedef struct {
    uint64_t state;
    uint64_t inc;   /* stream selector — must be odd */
} NqRand;

/* Seed (or re-seed) the generator.
 *   seed — any 64-bit value; deterministic: same seed+seq → same sequence.
 *   seq  — stream selector; two NqRand with the same seed but different seq
 *          values produce statistically independent streams. */
static inline void nq_rand_seed(NqRand *rng, uint64_t seed, uint64_t seq) {
    rng->state = 0u;
    rng->inc   = (seq << 1u) | 1u;   /* inc must be odd */
    /* Advance once to mix seed into state. */
    rng->state = rng->state * UINT64_C(6364136223846793005) + rng->inc;
    rng->state += seed;
    rng->state = rng->state * UINT64_C(6364136223846793005) + rng->inc;
}

/* Return the next 32-bit pseudo-random value (full range [0, 2^32)). */
static inline uint32_t nq_rand_u32(NqRand *rng) {
    uint64_t old = rng->state;
    rng->state = old * UINT64_C(6364136223846793005) + rng->inc;
    uint32_t xorshifted = (uint32_t)(((old >> 18u) ^ old) >> 27u);
    uint32_t rot = (uint32_t)(old >> 59u);
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31u));
}

/* Return a pseudo-random float in [0.0, 1.0). */
static inline float nq_rand_f(NqRand *rng) {
    /* Divide by 2^32, result in [0, 1). */
    return (float)(nq_rand_u32(rng) >> 8) * (1.0f / (float)(1u << 24));
}

/* Return a pseudo-random int in [lo, hi] (inclusive on both ends).
 * Undefined behaviour if lo > hi. Uses rejection sampling to avoid
 * modulo bias; terminates in O(1) expected iterations. */
static inline int nq_rand_range(NqRand *rng, int lo, int hi) {
    uint32_t range = (uint32_t)(hi - lo) + 1u;
    if (range == 0u) {
        /* Full 32-bit range requested; return raw value cast to int. */
        return (int)nq_rand_u32(rng);
    }
    /* Threshold for rejection: discard values < (2^32 % range) to avoid
     * bias towards lower numbers. */
    uint32_t threshold = (uint32_t)(-(int32_t)range) % range;
    uint32_t r;
    do {
        r = nq_rand_u32(rng);
    } while (r < threshold);
    return lo + (int)(r % range);
}

/* Return a pseudo-random float in [lo, hi). */
static inline float nq_rand_range_f(NqRand *rng, float lo, float hi) {
    return lo + nq_rand_f(rng) * (hi - lo);
}

/* Shuffle an array of `n` elements of `size` bytes using Fisher-Yates.
 * Uses a temporary buffer of `size` bytes on the stack; `size` must be
 * small (≤ 64 bytes) in practice for struct elements. */
#include <string.h>

static inline void nq_rand_shuffle(NqRand *rng, void *arr, int n, int size) {
    /* Avoid VLA: use a fixed-size temp buffer. Works for the common game
     * element sizes (int, float, pointer, small struct ≤ 64 bytes). */
    unsigned char tmp[64];
    unsigned char *base = (unsigned char *)arr;
    if (size > 64 || n <= 1) return;
    for (int i = n - 1; i > 0; i--) {
        int j = nq_rand_range(rng, 0, i);
        if (i == j) continue;
        memcpy(tmp,              base + (size_t)i * (size_t)size, (size_t)size);
        memcpy(base + (size_t)i * (size_t)size,
               base + (size_t)j * (size_t)size, (size_t)size);
        memcpy(base + (size_t)j * (size_t)size, tmp, (size_t)size);
    }
}

#endif /* NQ_RAND_H */
