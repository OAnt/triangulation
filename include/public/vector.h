#ifndef GEOMETRY_VECTOR_PUBLIC_H
#define GEOMETRY_VECTOR_PUBLIC_H

/**
 * Structure representing a 3D vector.
 */
struct vector_st {
    union {
        double v[3]; /** x, y and z values. */
        struct {
            double x, y, z;
        };
    };
};

#define VEC3(x, y, z) {{{(x), (y), (z)}}}
#define VEC2(x, y) VEC3(x, y, 0)

#endif
