#include <private/vector.h>

void vector_subtraction(
        _IN struct vector_st * in_a,
        _IN struct vector_st * in_b,
        _OUT struct vector_st * out)
{
    out->v[0] = in_a->v[0] - in_b->v[0];
    out->v[1] = in_a->v[1] - in_b->v[1];
    out->v[2] = in_a->v[2] - in_b->v[2];
}

void vector_addition(
        _IN struct vector_st * in_a,
        _IN struct vector_st * in_b,
        _OUT struct vector_st * out)
{
    out->v[0] = in_a->v[0] + in_b->v[0];
    out->v[1] = in_a->v[1] + in_b->v[1];
    out->v[2] = in_a->v[2] + in_b->v[2];
}

double vector_dot_product(
        _IN struct vector_st * in_a,
        _IN struct vector_st * in_b)
{
    return in_a->v[0] * in_b->v[0] + \
        in_a->v[1] * in_b->v[1] + \
        in_a->v[2] * in_b->v[2];
}

void vector_scale_by_scalar(
        _IN struct vector_st * in_a,
        _IN double scalar,
        _OUT struct vector_st * out)
{
    out->v[0] = in_a->v[0] * scalar;
    out->v[1] = in_a->v[1] * scalar;
    out->v[2] = in_a->v[2] * scalar;
}

enum point_position_e vector_position_relative_to_segment(
        _IN struct vector_st * point, 
        _IN struct segment_st * segment,
        _IN enum projection_plane_e d0)
{
    /* pp is the first dimension the next one is pp + 1, 
     * %3 is to convert (Z + 1) = 4 into 1 = X*/
    int d1 = (d0 + 1) % 3;
    double position = (segment->s[1].v[d0] - segment->s[0].v[d0]) *
        (point->v[d1] - segment->s[0].v[d1]) -
        (point->v[d0] - segment->s[0].v[d0]) *
        (segment->s[1].v[d1] - segment->s[0].v[d1]);
    if(position < 0){
        return pt_right;
    }else if(position > 0){
        return pt_left;
    }else{
        return pt_on;
    }
}

void vector_cross_product(
        _IN struct vector_st * in_a,
        _IN struct vector_st * in_b,
        _OUT struct vector_st * out)
{
    struct vector_st tmp;
    tmp.v[0] = in_a->v[1] * in_b->v[2] - in_a->v[2] * in_b->v[1];
    tmp.v[1] = in_a->v[2] * in_b->v[0] - in_a->v[0] * in_b->v[2];
    tmp.v[2] = in_a->v[0] * in_b->v[1] - in_a->v[1] * in_b->v[0];
    (*out) = tmp;
}

