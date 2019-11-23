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

