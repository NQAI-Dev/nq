#ifndef NQ_RAYCAST_H
#define NQ_RAYCAST_H

#include "nq/vec.h"
#include "nq/rect.h"
#include <stdbool.h>

/**
 * @brief Represents a ray for intersection testing.
 */
typedef struct nq_ray {
    NqVec2f origin;
    NqVec2f dir;     /**< Direction vector. Should be normalized. */
} nq_ray;

/**
 * @brief Represents the result of a raycast.
 */
typedef struct nq_raycast_hit {
    bool hit;        /**< True if intersection occurred. */
    NqVec2f point;   /**< Point of intersection. */
    NqVec2f normal;  /**< Surface normal at intersection point. */
    float distance;  /**< Distance from ray origin to intersection point. */
} nq_raycast_hit;

/**
 * @brief Tests intersection between a ray and an Axis-Aligned Bounding Box (AABB).
 * @param ray The ray.
 * @param aabb The rectangle representing the AABB.
 * @param max_dist Maximum distance to check.
 * @return Hit information.
 */
nq_raycast_hit nq_raycast_aabb(nq_ray ray, NqRect aabb, float max_dist);

/**
 * @brief Tests intersection between a ray and a circle.
 *
 * A ray that starts inside the circle reports an immediate hit at the
 * origin, with a normal opposite to the ray direction.
 *
 * @param ray The ray. Its direction should be normalized.
 * @param center Circle center.
 * @param radius Circle radius. A negative radius never hits.
 * @param max_dist Maximum distance to check. A negative limit never hits.
 * @return Hit information.
 */
nq_raycast_hit nq_raycast_circle(nq_ray ray, NqVec2f center,
                                  float radius, float max_dist);

#endif /* NQ_RAYCAST_H */
