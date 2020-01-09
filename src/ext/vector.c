#include <private/vector.h>

// This file contains parts of code, adapted and / or taken from other sources
// on the internet, it was not written by me and are not licensed under
// same terms as the rest of the code 


// #################
// taken from
// https://stackoverflow.com/questions/2752349/fast-rectangle-to-rectangle-intersection
// response by Daniel Vassallo (https://stackoverflow.com/users/222908/daniel-vassallo)
// Adapted to handle bottom left origin instead of top left (I think)
bool box_intersection_2D(
        const struct box_st * a,
        const struct box_st * b)
{
    return !(b->min.v[0] > a->max.v[0] ||
            b->max.v[0] < a->min.v[0] ||
            b->min.v[1] > a->max.v[1] ||
            b->max.v[1] < a->min.v[1]);
}
// #################

