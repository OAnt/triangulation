#ifndef GEOMETRY_PLANE_H
#define GEOMETRY_PLANE_H

#include <private/vector.h>

struct plane_st {
    double p[4];
};

double plane_vector_classify(
        struct plane_st * pl,
        struct vector_st * v);

#endif
