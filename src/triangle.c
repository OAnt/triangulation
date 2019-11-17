#include <private/triangle.h>
#include <private/common.h>

// computes the position of all 3 points composing the triangle 
// relative to the plane
struct vector_st _triangle_plane_classify(
        struct triangle_st * tr,
        struct plane_st * pl)
{
    struct vector_st clses;
    for(int i = 0; i < 3; i++){
        clses.v[i] = plane_vector_classify(pl, tr->t +i );
    }
    return clses;
}

// computes the classification of the triangle
// whose point positions regarding to plane are
// defined by the vector v
enum triangle_classification_e _vector_classify(
        struct vector_st * v)
{
    int all_coplanar = 1;
    int all_above = 1;
    int all_below = 1;
    for(int i = 0; i < 3; i++){
        double vec_cls = v->v[i];
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

enum triangle_classification_e triangle_plane_classify(
        struct triangle_st * tr,
        struct plane_st * pl)
{
    struct vector_st clses = _triangle_plane_classify(tr, pl);
    return _vector_classify(&clses);
}

