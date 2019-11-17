#ifndef GEOMETRY_PLANE_H
#define GEOMETRY_PLANE_H

#include <private/common.h>
#include <private/vector.h>

/**
 * Structure representing a 3D plane
 */
struct plane_st {
    double p[4]; /** plane equation is p[0] * x + p[1] * y + p[2] * z = p[3] */
};

/**
 * Computes the position of point regarding to the plane
 * param pl Plane to match against.
 * param v Vector representing the point
 * return 0 if the point lies on the plane.
 * A positive value if the point is above the plane.
 * A negative value if the point is below the plane.
 */
double plane_vector_classify(
        _IN struct plane_st * pl,
        _IN struct vector_st * v);

#endif
