#include <nq/rect.h>
#include "test_main.c"  /* for NQ_TEST_REGISTER / NQ_ASSERT_EQ / NQ_ASSERT */

static void test_rect_create(void) {
    NqRect r = nq_rect(10, 20, 30, 40);
    NQ_ASSERT_EQ(r.x, 10);
    NQ_ASSERT_EQ(r.y, 20);
    NQ_ASSERT_EQ(r.w, 30);
    NQ_ASSERT_EQ(r.h, 40);
}

static void test_rect_empty(void) {
    NQ_ASSERT(nq_rect_empty(nq_rect(0, 0, 0, 10)));
    NQ_ASSERT(nq_rect_empty(nq_rect(0, 0, 10, 0)));
    NQ_ASSERT(nq_rect_empty(nq_rect(0, 0, -5, 5)));
    NQ_ASSERT(!nq_rect_empty(nq_rect(0, 0, 1, 1)));
}

static void test_rect_contains(void) {
    NqRect r = nq_rect(10, 10, 20, 20); /* covers [10,30) x [10,30) */
    /* Inside corners + center */
    NQ_ASSERT(nq_rect_contains(r, nq_vec2i(10, 10))); /* top-left inclusive */
    NQ_ASSERT(nq_rect_contains(r, nq_vec2i(15, 15))); /* center */
    NQ_ASSERT(nq_rect_contains(r, nq_vec2i(29, 29))); /* just inside max edge */
    /* Outside corners */
    NQ_ASSERT(!nq_rect_contains(r, nq_vec2i(9, 15)));  /* left of */
    NQ_ASSERT(!nq_rect_contains(r, nq_vec2i(30, 15))); /* on max edge — half-open */
    NQ_ASSERT(!nq_rect_contains(r, nq_vec2i(15, 30)));
    NQ_ASSERT(!nq_rect_contains(r, nq_vec2i(31, 15)));
}

static void test_rect_intersects(void) {
    NqRect a = nq_rect(0, 0, 10, 10);
    NqRect b = nq_rect(5, 5, 10, 10);
    NqRect c = nq_rect(20, 20, 5, 5);
    NQ_ASSERT(nq_rect_intersects(a, b));
    NQ_ASSERT(nq_rect_intersects(b, a)); /* symmetric */
    NQ_ASSERT(!nq_rect_intersects(a, c));
    NQ_ASSERT(!nq_rect_intersects(c, a));
    /* Sharing an edge: half-open excludes overlap */
    NqRect d = nq_rect(10, 0, 10, 10); /* d.x == a.x + a.w */
    NQ_ASSERT(!nq_rect_intersects(a, d));
    /* Empty rect never intersects */
    NQ_ASSERT(!nq_rect_intersects(a, nq_rect(0, 0, 0, 0)));
}

static void test_rect_intersection(void) {
    NqRect a = nq_rect(0, 0, 10, 10);
    NqRect b = nq_rect(5, 5, 10, 10); /* overlap = [5,10) x [5,10) = 5x5 */
    NqRect i = nq_rect_intersection(a, b);
    NQ_ASSERT_EQ(i.x, 5);
    NQ_ASSERT_EQ(i.y, 5);
    NQ_ASSERT_EQ(i.w, 5);
    NQ_ASSERT_EQ(i.h, 5);
    /* Disjoint -> empty rect */
    NqRect j = nq_rect_intersection(a, nq_rect(20, 20, 5, 5));
    NQ_ASSERT(nq_rect_empty(j));
}

static void test_rect_union(void) {
    NqRect a = nq_rect(0, 0, 5, 5);
    NqRect b = nq_rect(10, 10, 5, 5);
    NqRect u = nq_rect_union(a, b); /* should be [0,15) x [0,15) */
    NQ_ASSERT_EQ(u.x, 0);
    NQ_ASSERT_EQ(u.y, 0);
    NQ_ASSERT_EQ(u.w, 15);
    NQ_ASSERT_EQ(u.h, 15);
    /* Union with empty = other rect */
    NqRect e = nq_rect_union(a, nq_rect(0, 0, 0, 0));
    NQ_ASSERT(nq_rect_eq(e, a));
}

static void test_rect_inflate(void) {
    NqRect r = nq_rect_inflate(nq_rect(10, 10, 20, 20), 5, 3);
    NQ_ASSERT_EQ(r.x, 5);
    NQ_ASSERT_EQ(r.y, 7);
    NQ_ASSERT_EQ(r.w, 30);
    NQ_ASSERT_EQ(r.h, 26);
}

NQ_TEST_REGISTER("rect_create",      test_rect_create);
NQ_TEST_REGISTER("rect_empty",        test_rect_empty);
NQ_TEST_REGISTER("rect_contains",     test_rect_contains);
NQ_TEST_REGISTER("rect_intersects",   test_rect_intersects);
NQ_TEST_REGISTER("rect_intersection", test_rect_intersection);
NQ_TEST_REGISTER("rect_union",        test_rect_union);
NQ_TEST_REGISTER("rect_inflate",      test_rect_inflate);

static void test_rect_center_basic(void) {
    /* 100x200 rect at (10, 20) → center at (60, 120) */
    NqVec2i c = nq_rect_center(nq_rect(10, 20, 100, 200));
    NQ_ASSERT_EQ(c.x, 60);
    NQ_ASSERT_EQ(c.y, 120);
}

static void test_rect_center_origin(void) {
    /* 4x4 rect at origin → center at (2, 2) — integer division */
    NqVec2i c = nq_rect_center(nq_rect(0, 0, 4, 4));
    NQ_ASSERT_EQ(c.x, 2);
    NQ_ASSERT_EQ(c.y, 2);
}

static void test_rect_center_empty(void) {
    /* Empty rect (w=0 or h=0) → returns the top-left corner */
    NQ_ASSERT(nq_vec2i_eq(nq_rect_center(nq_rect(50, 70, 0, 0)), nq_vec2i(50, 70)));
    /* Negative size: still returns top-left (well-defined even when
     * w/h is negative; useful so callers don't need to special-case) */
    NQ_ASSERT(nq_vec2i_eq(nq_rect_center(nq_rect(50, 70, -10, -10)), nq_vec2i(50, 70)));
}

NQ_TEST_REGISTER("rect_center_basic",  test_rect_center_basic);
NQ_TEST_REGISTER("rect_center_origin", test_rect_center_origin);
NQ_TEST_REGISTER("rect_center_empty",  test_rect_center_empty);

static void test_rect_circle_inside(void) {
    /* Circle completely inside the rect → 1 */
    NQ_ASSERT(nq_rect_contains_circle(nq_rect(0, 0, 100, 100), 50, 50, 5));
    /* Circle touching an edge from inside → 1 */
    NQ_ASSERT(nq_rect_contains_circle(nq_rect(0, 0, 100, 100), 5, 50, 5));
}

static void test_rect_circle_outside(void) {
    /* Circle clearly outside → 0 */
    NQ_ASSERT(!nq_rect_contains_circle(nq_rect(0, 0, 100, 100), -10, 50, 5));
    NQ_ASSERT(!nq_rect_contains_circle(nq_rect(0, 0, 100, 100), 150, 150, 5));
}

static void test_rect_circle_edge(void) {
    /* Circle just touching the rect from outside → 1 (inclusive)
     * Circle just past touching → 0 */
    NQ_ASSERT(nq_rect_contains_circle(nq_rect(0, 0, 100, 100), -5, 50, 5));   /* center at -5, radius 5 → touches edge at 0 */
    NQ_ASSERT(!nq_rect_contains_circle(nq_rect(0, 0, 100, 100), -6, 50, 5));  /* center at -6, radius 5 → 1px outside */
}

static void test_rect_circle_corner(void) {
    /* Circle in the corner — closest point on rect is the corner (0,0),
     * so distance² to (0,0) should be <= radius². */
    NQ_ASSERT(nq_rect_contains_circle(nq_rect(0, 0, 100, 100), -3, -3, 5));   /* dist² = 18, radius² = 25 → inside */
    NQ_ASSERT(!nq_rect_contains_circle(nq_rect(0, 0, 100, 100), -5, -5, 5));  /* dist² = 50, radius² = 25 → outside */
}

static void test_rect_circle_empty_or_zero_radius(void) {
    /* Empty rect → always no overlap */
    NQ_ASSERT(!nq_rect_contains_circle(nq_rect(10, 10, 0, 0), 10, 10, 5));
    /* Zero radius → behaves like contains() */
    NQ_ASSERT(nq_rect_contains_circle(nq_rect(0, 0, 100, 100), 50, 50, 0));
    NQ_ASSERT(!nq_rect_contains_circle(nq_rect(0, 0, 100, 100), -5, -5, 0));
}

NQ_TEST_REGISTER("rect_circle_inside",          test_rect_circle_inside);
NQ_TEST_REGISTER("rect_circle_outside",         test_rect_circle_outside);
NQ_TEST_REGISTER("rect_circle_edge",            test_rect_circle_edge);
NQ_TEST_REGISTER("rect_circle_corner",          test_rect_circle_corner);
NQ_TEST_REGISTER("rect_circle_empty_or_zero",  test_rect_circle_empty_or_zero_radius);

static void test_circle_overlap_overlapping_returns_true(void) {
    /* Two overlapping circles → 1 */
    NQ_ASSERT(nq_circle_overlap(0, 0, 5, 6, 0, 5));   /* dist=6, r_sum=10, overlap */
    NQ_ASSERT(nq_circle_overlap(0, 0, 10, 5, 5, 10)); /* concentric distance 7, r_sum=20, overlap */
}

static void test_circle_overlap_disjoint_returns_false(void) {
    /* Two far-apart circles → 0 */
    NQ_ASSERT(!nq_circle_overlap(0, 0, 5, 100, 100, 5));   /* dist²=20000, r_sum=10 */
    NQ_ASSERT(!nq_circle_overlap(-50, 0, 5, 50, 0, 5));   /* dist²=10000, r_sum=10 */
}

static void test_circle_overlap_touching_returns_true(void) {
    /* Two circles exactly touching → 1 (inclusive) */
    NQ_ASSERT(nq_circle_overlap(0, 0, 5, 10, 0, 5));  /* dist=10, r_sum=10 */
}

static void test_circle_overlap_zero_radius(void) {
    /* Zero/negative radius behaves like a point (radius 0) */
    NQ_ASSERT(nq_circle_overlap(0, 0, 0, 0, 0, 0));  /* two points at origin */
    NQ_ASSERT(nq_circle_overlap(0, 0, 0, 1, 0, 0));  /* points apart → no overlap */
    NQ_ASSERT(nq_circle_overlap(0, 0, -5, 3, 0, 0)); /* negative radius → treated as 0 */
}

NQ_TEST_REGISTER("circle_overlap_overlapping",   test_circle_overlap_overlapping_returns_true);
NQ_TEST_REGISTER("circle_overlap_disjoint",      test_circle_overlap_disjoint_returns_false);
NQ_TEST_REGISTER("circle_overlap_touching",      test_circle_overlap_touching_returns_true);
NQ_TEST_REGISTER("circle_overlap_zero_radius",   test_circle_overlap_zero_radius);

static void test_circle_overlap_f_overlapping(void) {
    /* Float version: same semantics as int, just on floats */
    NQ_ASSERT(nq_circle_overlap_f(0.0f, 0.0f, 5.0f, 6.0f, 0.0f, 5.0f));
    NQ_ASSERT(nq_circle_overlap_f(0.5f, 0.5f, 1.0f, 1.0f, 1.5f, 1.0f));
}

static void test_circle_overlap_f_touching(void) {
    /* Distance² == r_sum² → touching, inclusive */
    NQ_ASSERT(nq_circle_overlap_f(0.0f, 0.0f, 5.0f, 10.0f, 0.0f, 5.0f));
}

static void test_circle_overlap_f_disjoint(void) {
    NQ_ASSERT(!nq_circle_overlap_f(0.0f, 0.0f, 5.0f, 100.0f, 100.0f, 5.0f));
}

static void test_circle_overlap_f_negative_radius(void) {
    /* Negative radius clamped to 0 */
    NQ_ASSERT(nq_circle_overlap_f(0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f));
}

NQ_TEST_REGISTER("circle_overlap_f_overlapping",   test_circle_overlap_f_overlapping);
NQ_TEST_REGISTER("circle_overlap_f_touching",      test_circle_overlap_f_touching);
NQ_TEST_REGISTER("circle_overlap_f_disjoint",      test_circle_overlap_f_disjoint);
NQ_TEST_REGISTER("circle_overlap_f_neg_radius",    test_circle_overlap_f_negative_radius);

static void test_rect_contains_circle_f_inside(void) {
    /* Float circle fully inside the rect → 1 */
    NQ_ASSERT(nq_rect_contains_circle_f(nq_rect(0, 0, 100, 100), 50.0f, 50.0f, 5.0f));
}

static void test_rect_contains_circle_f_outside(void) {
    /* Float circle clearly outside the rect → 0 */
    NQ_ASSERT(!nq_rect_contains_circle_f(nq_rect(0, 0, 100, 100), 200.0f, 200.0f, 5.0f));
}

static void test_rect_contains_circle_f_corner(void) {
    /* Float circle in the corner — closest point on rect is the corner */
    NQ_ASSERT(nq_rect_contains_circle_f(nq_rect(0, 0, 100, 100), -3.0f, -3.0f, 5.0f));   /* radius²=25, dist²=18 */
    NQ_ASSERT(!nq_rect_contains_circle_f(nq_rect(0, 0, 100, 100), -5.0f, -5.0f, 5.0f));  /* dist²=50, > 25 */
}

static void test_rect_contains_circle_f_zero_radius(void) {
    /* Zero radius → behaves like contains() */
    NQ_ASSERT(nq_rect_contains_circle_f(nq_rect(0, 0, 100, 100), 50.0f, 50.0f, 0.0f));
    NQ_ASSERT(!nq_rect_contains_circle_f(nq_rect(0, 0, 100, 100), 200.0f, 200.0f, 0.0f));
}

NQ_TEST_REGISTER("rect_contains_circle_f_inside",     test_rect_contains_circle_f_inside);
NQ_TEST_REGISTER("rect_contains_circle_f_outside",    test_rect_contains_circle_f_outside);
NQ_TEST_REGISTER("rect_contains_circle_f_corner",     test_rect_contains_circle_f_corner);
NQ_TEST_REGISTER("rect_contains_circle_f_zero_radius", test_rect_contains_circle_f_zero_radius);

static void test_rect_intersect_circle_overlap_inside(void) {
    /* Circle centred inside a 100x100 rect with small radius:
     * the overlap is the circle's bounding box. */
    NqRect r = nq_rect(0, 0, 100, 100);
    NqRect o = nq_rect_intersect_circle(r, 50, 50, 5);
    /* Circle AABB is (45, 45, 10, 10), all contained inside r. */
    NQ_ASSERT(nq_rect_eq(o, nq_rect(45, 45, 10, 10)));
}

static void test_rect_intersect_circle_partial_overlap(void) {
    /* Circle pokes out of the rect on the right side. Overlap is the
     * circle's AABB clipped to r. */
    NqRect r = nq_rect(0, 0, 100, 100);
    NqRect o = nq_rect_intersect_circle(r, 105, 50, 10);
    /* Circle AABB is (95, 40, 20, 20), clipped to r → (95, 40, 5, 20). */
    NQ_ASSERT(o.x == 95);
    NQ_ASSERT(o.y == 40);
    NQ_ASSERT(o.w == 5);
    NQ_ASSERT(o.h == 20);
}

static void test_rect_intersect_circle_no_overlap(void) {
    /* Circle completely outside the rect → empty overlap */
    NqRect r = nq_rect(0, 0, 100, 100);
    NqRect o = nq_rect_intersect_circle(r, 200, 200, 5);
    NQ_ASSERT(o.w <= 0 || o.h <= 0);
}

static void test_rect_intersect_circle_zero_radius(void) {
    /* Zero radius → behaves like a point intersection */
    NqRect r = nq_rect(0, 0, 100, 100);
    NqRect o = nq_rect_intersect_circle(r, 50, 50, 0);
    NQ_ASSERT(nq_rect_eq(o, nq_rect(50, 50, 0, 0)));
    /* Point outside the rect → empty */
    o = nq_rect_intersect_circle(r, 200, 200, 0);
    NQ_ASSERT(o.w <= 0 || o.h <= 0);
}

NQ_TEST_REGISTER("rect_intersect_circle_inside",      test_rect_intersect_circle_overlap_inside);
NQ_TEST_REGISTER("rect_intersect_circle_partial",     test_rect_intersect_circle_partial_overlap);
NQ_TEST_REGISTER("rect_intersect_circle_no_overlap",   test_rect_intersect_circle_no_overlap);
NQ_TEST_REGISTER("rect_intersect_circle_zero_radius", test_rect_intersect_circle_zero_radius);

static void test_rect_intersect_circle_f_inside(void) {
    NqRect r = nq_rect(0, 0, 100, 100);
    NqRect o = nq_rect_intersect_circle_f(r, 50.5f, 50.5f, 5.0f);
    /* (int)50.5 - 5 = 45; 2*5 = 10; clipped to r → (45, 45, 10, 10). */
    NQ_ASSERT(nq_rect_eq(o, nq_rect(45, 45, 10, 10)));
}

static void test_rect_intersect_circle_f_partial(void) {
    NqRect r = nq_rect(0, 0, 100, 100);
    /* Circle centred slightly past right edge */
    NqRect o = nq_rect_intersect_circle_f(r, 105.0f, 50.0f, 10.0f);
    /* Circle AABB clipped: (105-10, 50-10, 20, 20) → (95, 40, 5, 20). */
    NQ_ASSERT(o.x == 95);
    NQ_ASSERT(o.y == 40);
    NQ_ASSERT(o.w == 5);
    NQ_ASSERT(o.h == 20);
}

static void test_rect_intersect_circle_f_no_overlap(void) {
    NqRect r = nq_rect(0, 0, 100, 100);
    NqRect o = nq_rect_intersect_circle_f(r, 200.0f, 200.0f, 5.0f);
    NQ_ASSERT(o.w <= 0 || o.h <= 0);
}

NQ_TEST_REGISTER("rect_intersect_circle_f_inside",    test_rect_intersect_circle_f_inside);
NQ_TEST_REGISTER("rect_intersect_circle_f_partial",   test_rect_intersect_circle_f_partial);
NQ_TEST_REGISTER("rect_intersect_circle_f_no_overlap", test_rect_intersect_circle_f_no_overlap);

static void test_circle_penetration_separates_two_circles(void) {
    /* Two overlapping circles centred 6 apart, each radius 5: overlap = 4.
     * Vector should be 4 units along +X (the axis from A to B). */
    NqVec2f v = nq_circle_penetration_vector_f(0.0f, 0.0f, 5.0f,
                                                6.0f, 0.0f, 5.0f);
    /* |v| should equal overlap (4); direction +X (so v.x > 0, v.y == 0) */
    NQ_ASSERT(v.y == 0.0f);
    float mag = sqrtf(v.x * v.x + v.y * v.y);
    NQ_ASSERT(mag > 3.99f && mag < 4.01f);
    NQ_ASSERT(v.x > 0.0f);
}

static void test_circle_penetration_no_overlap_returns_zero(void) {
    /* Two disjoint circles → zero vector. */
    NqVec2f v = nq_circle_penetration_vector_f(0.0f, 0.0f, 5.0f,
                                                100.0f, 0.0f, 5.0f);
    NQ_ASSERT(v.x == 0.0f);
    NQ_ASSERT(v.y == 0.0f);
}

static void test_circle_penetration_concentric_pushes_x(void) {
    /* Concentric circles (A and B at same point, both radius 5).
     * Returns a positive-x vector of length (ar + br) = 10. */
    NqVec2f v = nq_circle_penetration_vector_f(5.0f, 5.0f, 5.0f,
                                                5.0f, 5.0f, 5.0f);
    NQ_ASSERT(v.y == 0.0f);
    NQ_ASSERT(v.x == 10.0f);
}

static void test_circle_penetration_int_variant(void) {
    /* Integer variant delegates to float. Verify same behavior. */
    NqVec2f v = nq_circle_penetration_vector(0, 0, 5,
                                             4, 3, 5);
    NQ_ASSERT(v.y != 0.0f);  /* centre B is at (4, 3), so y component non-zero */
    /* magnitude = overlap = (5+5) - sqrt(4*4 + 3*3) = 10 - 5 = 5 */
    float mag = sqrtf(v.x * v.x + v.y * v.y);
    NQ_ASSERT(mag > 4.99f && mag < 5.01f);
}

NQ_TEST_REGISTER("circle_penetration_separates",      test_circle_penetration_separates_two_circles);
NQ_TEST_REGISTER("circle_penetration_no_overlap",    test_circle_penetration_no_overlap_returns_zero);
NQ_TEST_REGISTER("circle_penetration_concentric",    test_circle_penetration_concentric_pushes_x);
NQ_TEST_REGISTER("circle_penetration_int_variant",    test_circle_penetration_int_variant);

static void test_rect_penetration_circle_outside(void) {
    /* Circle clearly outside the rect, on the right side */
    NqRect r = nq_rect(0, 0, 100, 100);
    NqVec2f v = nq_rect_penetration_vector_f(r, 120.0f, 50.0f, 10.0f);
    /* Closest point on rect = (100, 50). Distance = 20. Push -X by (10 - 20) = -10. So v should be (-10, 0). */
    NQ_ASSERT(v.x == -10.0f);
    NQ_ASSERT(v.y == 0.0f);
}

static void test_rect_penetration_circle_no_overlap(void) {
    /* Circle completely outside rect → zero vector */
    NqRect r = nq_rect(0, 0, 100, 100);
    NqVec2f v = nq_rect_penetration_vector_f(r, 200.0f, 200.0f, 10.0f);
    NQ_ASSERT(v.x == 0.0f);
    NQ_ASSERT(v.y == 0.0f);
}

static void test_rect_penetration_circle_pokes_corner(void) {
    /* Circle centred at (-3, -3) with radius 5 — closest point is the
     * rect's corner (0, 0). Distance² = 18, radius² = 25, so dist = √18 ≈ 4.24,
     * overlap = 5 - 4.24 ≈ 0.76. Push direction is from corner outward
     * (away from rect), so vector is (-(-3)/4.24 * 0.76, -(-3)/4.24 * 0.76)
     * ≈ (-0.54, -0.54). */
    NqRect r = nq_rect(0, 0, 100, 100);
    NqVec2f v = nq_rect_penetration_vector_f(r, -3.0f, -3.0f, 5.0f);
    NQ_ASSERT(v.x < 0.0f);
    NQ_ASSERT(v.y < 0.0f);
    /* Both components should have the same magnitude */
    NQ_ASSERT(v.x == v.y);
}

static void test_rect_penetration_circle_inside_pushes_axis(void) {
    /* Circle centred inside the rect */
    NqRect r = nq_rect(0, 0, 100, 100);
    NqVec2f v = nq_rect_penetration_vector_f(r, 50.0f, 50.0f, 10.0f);
    /* Push along the shorter axis to the nearest edge.
     * Distance to nearest edge: left=50, right=50, top=50, bottom=50.
     * All equal — picks left (-1, 0). Push magnitude = 50 + 10 = 60. */
    NQ_ASSERT(v.y == 0.0f);
    NQ_ASSERT(v.x == -60.0f);
}

NQ_TEST_REGISTER("rect_penetration_circle_outside", test_rect_penetration_circle_outside);
NQ_TEST_REGISTER("rect_penetration_circle_no_overlap", test_rect_penetration_circle_no_overlap_returns_zero);
NQ_TEST_REGISTER("rect_penetration_circle_corner", test_rect_penetration_circle_pokes_corner);
NQ_TEST_REGISTER("rect_penetration_circle_inside", test_rect_penetration_circle_inside_pushes_axis);
