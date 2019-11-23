#ifndef GEOMETRY_VECTOR_H
#define GEOMETRY_VECTOR_H

#include <private/common.h>

/**
 * Structure representing a 3D vector.
 */
struct vector_st {
    double v[3]; /** x, y and z values */
};

/** Computes the subtraction in_a - in_b.
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
 * Computes the dot product between in_a and in_b.
 * Works even if out points to either in_a or in_b,
 * or both.
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

#endif
