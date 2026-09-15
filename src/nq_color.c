#include "nq/color.h"

NqColor nq_color_lerp(NqColor a, NqColor b, float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return NQ_COLOR_RGBA(
        (uint8_t)(a.r + (b.r - a.r) * t),
        (uint8_t)(a.g + (b.g - a.g) * t),
        (uint8_t)(a.b + (b.b - a.b) * t),
        (uint8_t)(a.a + (b.a - a.a) * t)
    );
}

NqColor nq_color_from_uint32(uint32_t rgba) {
    return NQ_COLOR_RGBA(
        (uint8_t)((rgba >> 24) & 0xFF),
        (uint8_t)((rgba >> 16) & 0xFF),
        (uint8_t)((rgba >>  8) & 0xFF),
        (uint8_t)( rgba        & 0xFF)
    );
}
