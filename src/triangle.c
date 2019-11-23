#include "public/common.h"
#include <private/triangle.h>

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

/*
 * Computes out = (in_a . in_b) x in_c
 */
void vector_scale_by_dot_product(
        struct vector_st * in_a,
        struct vector_st * in_b,
        struct vector_st * in_c,
        struct vector_st * out)
{
    double ab_dot_product = vector_dot_product(in_a, in_b);
    vector_scale_by_scalar(in_c, ab_dot_product, out);
}

/*
 * Computes out = in_a x (in_b x in_c)
 * which is equal to (in_a . in_c) x in_b - (in_a . in_b) x in_c
 */
void vector_three_way_cross_product_bc_first(
        struct vector_st * in_a,
        struct vector_st * in_b,
        struct vector_st * in_c,
        struct vector_st * out)
{
    struct vector_st scaled_b;
    vector_scale_by_dot_product(in_a, in_c, in_b, &scaled_b);
    struct vector_st scaled_c;
    vector_scale_by_dot_product(in_a, in_b, in_c, &scaled_c);
    vector_subtraction(&scaled_b, &scaled_c, out);
}

/* 
 * computation taken form wikipedia:
 * https://en.wikipedia.org/wiki/Circumscribed_circle
 */
enum error_code_e triangle_compute_circumcircle_center(
        struct triangle_st * tr,
        struct vector_st * cc_center)
{
    // tr is considered the ABC triangle
    //compute a = A - C;
    struct vector_st a;
    vector_subtraction(tr->t, tr->t + 2, &a);
    double sq_a_len = vector_dot_product(&a, &a);
    //compute b = B - C;
    struct vector_st b;
    vector_subtraction(tr->t + 1, tr->t + 2, &b);
    double sq_b_len = vector_dot_product(&a, &a);
    // computing ||b|| * ||b|| x a
    struct vector_st scaled_a;
    vector_scale_by_scalar(&a, sq_b_len, &scaled_a);
    // computing ||a|| * ||a|| x b
    struct vector_st scaled_b;
    vector_scale_by_scalar(&b, sq_a_len, &scaled_b);
    // computing ||a|| * ||a|| x b - ||b|| * ||b|| x a
    struct vector_st scaled_diff;
    vector_subtraction(&scaled_b, &scaled_a, &scaled_diff);
    //computing (||a|| * ||a|| x b - ||b|| * ||b|| x a) x (a x b)
    struct vector_st unscaled_direction;
    vector_three_way_cross_product_bc_first(
            &scaled_diff, &a, &b, &unscaled_direction);
    // computing 2 * ||a x b|| * ||a x b||
    // ||a x b|| = sqrt(||a||*||a|| * ||b||*||b|| - (a . b)*(a . b))
    double ab_dot_product = vector_dot_product(&a, &b);
    double sq_ab_cross_prdt_len = sq_a_len * sq_b_len - \
                                  ab_dot_product * ab_dot_product;
    double denom = 2 * sq_ab_cross_prdt_len;
    if(denom == 0.0) return ec_div_by_zero_error;
    // computing the relative cc center
    struct vector_st relative_cc_center;
    vector_scale_by_scalar(
            &unscaled_direction, 1 / denom, &relative_cc_center);
    vector_addition(&relative_cc_center, tr->t + 2, cc_center);
    return ec_no_error;
}


