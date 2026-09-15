/*
 * nq — color utilities.
 *
 * `nq_color_lerp` interpolates two RGBA8 colors by a factor `t` in [0,1].
 * Each channel (R, G, B, A) is interpolated independently. Out-of-range t
 * is clamped (matching `nq_ease` behaviour so colour lerps compose
 * with easing functions without surprising results).
 *
 * `nq_color_from_uint32` packs a 32-bit 0xRRGGBBAA value into an
 * NqColor without the caller having to extract channels by hand.
 */
#ifndef NQ_COLOR_H
#define NQ_COLOR_H

#include <stdint.h>

#include "nq/graphics.h"  /* NqColor, NQ_COLOR_RGB / NQ_COLOR_RGBA macros */

NqColor nq_color_lerp(NqColor a, NqColor b, float t);
NqColor nq_color_from_uint32(uint32_t rgba);

#endif /* NQ_COLOR_H */
