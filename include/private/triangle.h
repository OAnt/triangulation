#ifndef GEOMETRY_TRIANGLE_H
#define GEOMETRY_TRIANGLE_H

#include <private/vector.h>
#include <private/plane.h>

/**
 * Structure representing a 3D triangle.
 */
struct triangle_st {
    struct vector_st t[3]; /** the 3 vertices representing the triangle. */
};

/**
 * Position of a triangle relative to plane.
 */
enum triangle_classification_e {
    trcl_above, /** The triangle is above the plane. */
    trcl_below, /** The triangle is below the plane. */
    trcl_intersected, /** The plane intersects the triangle. */
    trcl_coplanar, /** The triangle and the plane are coplanar. */
};

/**
 * Computes the position of a triangle relative to a plane.
 * param tr Triangle to classify.
 * param pl The triangle position will be computed relative to
 * this plane.
 * return The position of tr relative to pl.
 */
enum triangle_classification_e triangle_plane_classify(
        struct triangle_st * tr,
        struct plane_st * pl);

#endif
