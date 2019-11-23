#ifndef GEOMETRY_SIGN_H
#define GEOMETRY_SIGN_H

#include <public/common.h>

#define SIGN(x) ((x) > 0) - ((x) < 0)
#define MAX(x, y) (x) > (y) ? x : y
#define MIN(x, y) (x) < (y) ? x : y
#define EPSILON 1e-10

#endif
