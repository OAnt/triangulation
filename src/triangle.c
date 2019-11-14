#include <private/triangle.h>
#include <private/common.h>

enum triangle_classification_e triangle_plane_classify(
        struct triangle_st * tr,
        struct plane_st * pl)
{
    int all_coplanar = 1;
    int all_above = 1;
    int all_below = 1;
    for(int i = 0; i < 3; i++){
        double vec_cls = plane_vector_classify(pl, tr->t + i);
        all_coplanar = all_coplanar && (vec_cls == 0);
        all_above = all_above && (vec_cls >= 0);
        all_below = all_below && (vec_cls <= 0);
    }
    if(all_coplanar){
        return trcl_coplanar;
    }else if(all_above){
        return trcl_above;
    }else if(all_below){
        return trcl_below;
    }else{
        return trcl_intersected;    
    }
}

