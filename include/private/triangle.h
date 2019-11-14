#ifndef GEOMETRY_TRIANGLE_H
#define GEOMETRY_TRIANGLE_H

#include <private/vector.h>
#include <private/plane.h>

struct triangle_st {
    struct vector_st t[3];
};

enum triangle_classification_e {
    trcl_above,
    trcl_below,
    trcl_intersected,
    trcl_coplanar,
};

enum triangle_classification_e triangle_plane_classify(
        struct triangle_st * tr,
        struct plane_st * pl);

#endif
