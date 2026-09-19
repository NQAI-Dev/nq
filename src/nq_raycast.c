#include "nq/raycast.h"
#include <math.h>
#include <float.h>

nq_raycast_hit nq_raycast_aabb(nq_ray ray, NqRect aabb, float max_dist) {
    nq_raycast_hit result = {
        .hit = false,
        .point = {0.0f, 0.0f},
        .normal = {0.0f, 0.0f},
        .distance = 0.0f
    };

    float tmin = -FLT_MAX;
    float tmax = FLT_MAX;
    
    NqVec2f min = {aabb.x, aabb.y};
    NqVec2f max = {aabb.x + aabb.w, aabb.y + aabb.h};
    
    NqVec2f normal_min = {0.0f, 0.0f};

    if (fabsf(ray.dir.x) < 1e-6f) {
        if (ray.origin.x < min.x || ray.origin.x > max.x) {
            return result;
        }
    } else {
        float inv_dir = 1.0f / ray.dir.x;
        float t1 = (min.x - ray.origin.x) * inv_dir;
        float t2 = (max.x - ray.origin.x) * inv_dir;
        
        NqVec2f n1 = {-1.0f, 0.0f};
        NqVec2f n2 = {1.0f, 0.0f};
        
        if (t1 > t2) {
            float temp = t1; t1 = t2; t2 = temp;
            NqVec2f temp_n = n1; n1 = n2; n2 = temp_n;
        }
        
        if (t1 > tmin) {
            tmin = t1;
            normal_min = n1;
        }
        if (t2 < tmax) {
            tmax = t2;
        }
        if (tmin > tmax) return result;
    }
    
    if (fabsf(ray.dir.y) < 1e-6f) {
        if (ray.origin.y < min.y || ray.origin.y > max.y) {
            return result;
        }
    } else {
        float inv_dir = 1.0f / ray.dir.y;
        float t1 = (min.y - ray.origin.y) * inv_dir;
        float t2 = (max.y - ray.origin.y) * inv_dir;
        
        NqVec2f n1 = {0.0f, -1.0f};
        NqVec2f n2 = {0.0f, 1.0f};
        
        if (t1 > t2) {
            float temp = t1; t1 = t2; t2 = temp;
            NqVec2f temp_n = n1; n1 = n2; n2 = temp_n;
        }
        
        if (t1 > tmin) {
            tmin = t1;
            normal_min = n1;
        }
        if (t2 < tmax) {
            tmax = t2;
        }
        if (tmin > tmax) return result;
    }
    
    if (tmax < 0.0f) {
        return result;
    }
    
    float t = tmin;
    NqVec2f best_normal = normal_min;
    
    if (tmin < 0.0f) {
        t = 0.0f;
        best_normal.x = -ray.dir.x;
        best_normal.y = -ray.dir.y;
        float len = sqrtf(best_normal.x * best_normal.x + best_normal.y * best_normal.y);
        if (len > 0.0f) {
            best_normal.x /= len;
            best_normal.y /= len;
        }
    }
    
    if (t > max_dist) {
        return result;
    }
    
    result.hit = true;
    result.distance = t;
    result.point.x = ray.origin.x + ray.dir.x * t;
    result.point.y = ray.origin.y + ray.dir.y * t;
    result.normal = best_normal;
    
    return result;
}


nq_raycast_hit nq_raycast_circle(nq_ray ray, NqVec2f center,
                                  float radius, float max_dist) {
    nq_raycast_hit result = {
        .hit = false,
        .point = {0.0f, 0.0f},
        .normal = {0.0f, 0.0f},
        .distance = 0.0f
    };
    float offset_x;
    float offset_y;
    float direction_len_sq;
    float projection;
    float discriminant;
    float t;

    if (radius < 0.0f || max_dist < 0.0f) {
        return result;
    }

    offset_x = ray.origin.x - center.x;
    offset_y = ray.origin.y - center.y;
    direction_len_sq = ray.dir.x * ray.dir.x + ray.dir.y * ray.dir.y;

    if (offset_x * offset_x + offset_y * offset_y <= radius * radius) {
        float direction_len = sqrtf(direction_len_sq);

        result.hit = true;
        result.point = ray.origin;
        if (direction_len > 0.0f) {
            result.normal.x = -ray.dir.x / direction_len;
            result.normal.y = -ray.dir.y / direction_len;
        }
        return result;
    }

    if (direction_len_sq <= 0.0f) {
        return result;
    }

    projection = offset_x * ray.dir.x + offset_y * ray.dir.y;
    discriminant = projection * projection - direction_len_sq *
                   (offset_x * offset_x + offset_y * offset_y - radius * radius);
    if (discriminant < 0.0f) {
        return result;
    }

    t = (-projection - sqrtf(discriminant)) / direction_len_sq;
    if (t < 0.0f || t > max_dist) {
        return result;
    }

    result.hit = true;
    result.distance = t;
    result.point.x = ray.origin.x + ray.dir.x * t;
    result.point.y = ray.origin.y + ray.dir.y * t;
    if (radius > 0.0f) {
        result.normal.x = (result.point.x - center.x) / radius;
        result.normal.y = (result.point.y - center.y) / radius;
    }
    return result;
}
