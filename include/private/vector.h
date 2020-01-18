#ifndef GEOMETRY_VECTOR_H
#define GEOMETRY_VECTOR_H

#include <stdbool.h>
#include <stdint.h>
#include <private/common.h>
#include <public/vector.h>

/** 
 * Structure representing a 3D box.
 */
struct box_st{
    struct vector_st min; /** Bottom left point. */
    struct vector_st max; /** Top right point. */
};

#define BOX2(x1, y1, x2, y2) {VEC2(x1, y1), VEC2(x2, y2)}
#define BOX3(x1, y1, z1, x2, y2, z2) {VEC3(x1, y1, z1), VEC3(x2, y2, z2)}

#define box_size_along(box, axis) (box).max.v[(axis)] - (box).min.v[(axis)]

/** Computes the subtraction in_a - in_b.
 * Works even if out points to either in_a or in_b,
 * or both.
 * param in_a Vector to subtract b to.
 * param in_b Vector subtracted from a.
 * param out Vector a - b
 * return nothing
 */
void vector_subtraction(
        _IN struct vector_st * in_a,
        _IN struct vector_st * in_b,
        _OUT struct vector_st * out);

/**
 * Sums in_a and in_b into out.
 * Works even if out points to either in_a or in_b,
 * or both.
 * param in_a Vector to add to in_b
 * param in_b Vector to add to in_&
 * param out Addition result
 * return nothing, does not fail.
 */
void vector_addition(
        _IN struct vector_st * in_a,
        _IN struct vector_st * in_b,
        _OUT struct vector_st * out);

/** 
 * Computes the dot product between in_a and in_b.
 * param in_a Vector.
 * param in_b Vector.
 * return The dot product between in_a and in_b
 */
double vector_dot_product(
        _IN struct vector_st * in_a,
        _IN struct vector_st * in_b);

/**
 * Scales the vector by a scalar. Works even if out
 * points to in_a.
 * param in_a Vector to scale.
 * param scalar Scaling parameter.
 * param out Resulting vector.
 * return nothing, does not fail.
 */
void vector_scale_by_scalar(
        _IN struct vector_st * in_a,
        _IN double scalar,
        _OUT struct vector_st * out);

/**
 * Line segment
 */
struct segment_st{
    struct vector_st s[2]; /** line segment extremities*/
};

/**
 * Position of a point relative to a segment.
 */
enum point_position_e {
    pt_left, /** Point is on the left of the segment */
    pt_right, /** Point is on the right of the segment */
    pt_on /** Point is on the line supported by the segment */
};
/**
 * Computes the position of a point relative to a segment.
 * param point Point for which the position is to be computed.
 * param seg0 First point of the segment relative to which the point is to be positioned.
 * param seg1 Second point of the segment relative to which the point is to be positioned.
 * param x Index of the axis that should be considered as first.
 * param y Index of the axis that should be considered as second. System must
 * be direct.
 * return The point position.
 */
enum point_position_e _vector_position_relative_to_segment(
        _IN struct vector_st * point, 
        _IN struct vector_st * seg0, 
        _IN struct vector_st * seg1, 
        _IN int32_t x,
        _IN int32_t y);

/**
 * Computes the position of a point relative to a segment.
 * param point Point for which the position is to be computed.
 * param segment Segment relative to which the point is to be positioned.
 * param projection_plane Projection plane to use.
 * return The point position.
 */
enum point_position_e vector_position_relative_to_segment(
        _IN struct vector_st * point, 
        _IN struct segment_st * segment,
        _IN enum projection_plane_e projection_plane);

/**
 * Computes the cross product between to vectors.
 * param in_a First member of the cross product.
 * param in_b Second member of the cross product.
 * param out Result of the cross product.
 * return Nothing.
 */
void vector_cross_product(
        _IN struct vector_st * in_a,
        _IN struct vector_st * in_b,
        _OUT struct vector_st * out);

/** Determines the axis system for a given projection plane.
 * param pp Desired projection plane.
 * param x Storage for the index of the first axis in the new system.
 * param y Storage for the index of the second axis in the new
 * system.
 * return nothing
 */
void get_axis_system_from_projection_plane(
        _IN enum projection_plane_e pp,
        _OUT int32_t * x,
        _OUT int32_t * y);

/**
 * Tells if two 2D boxes intersects (only x and y coordinate)
 * param a First box
 * param b Second box
 * return true if there is an intersection, false otherwise.
 */
bool box_intersection_2D(
        const struct box_st * a,
        const struct box_st * b);

#endif
