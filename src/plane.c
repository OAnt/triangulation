#include "private/vector.h"
#include <private/plane.h>

double plane_vector_classify(
        struct plane_st * pl,
        struct vector_st * v)
{
    return pl->p[0] * v->v[0] + pl->p[1] * v->v[1] + \
        pl->p[2] * v->v[2] - pl->p[3];
}

