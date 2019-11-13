#include <private/triangle.h>

enum triangle_classification_e {
    trcl_above,
    trcl_below,
    trcl_intersected,
    trcl_coplanar,
};

enum triangle_classification_e triangle_plane_classify(
        struct triangle_st * tr,
        struct plane_st * pl)
{
    return trcl_intersected;    
}

