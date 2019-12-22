#ifndef GEOMETRY_SIGN_H
#define GEOMETRY_SIGN_H

#include <public/common.h>

#define SIGN(x) ((x) > 0) - ((x) < 0)
#define MAX(x, y) (x) > (y) ? x : y
#define MIN(x, y) (x) < (y) ? x : y
#define EPSILON 1e-10
// When debug choosing a larger invalid index
// In case of arrays, -1 is still within the
// allocated area
#ifndef NDEBUG
#define INVALID_INDEX (size_t)-30
#else
#define INVALID_INDEX (size_t)-1
#endif

#define USE_PREDICATES 

#endif
