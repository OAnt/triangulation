#ifndef GEOMETRY_ASSERT_H
#define GEOMETRY_ASSERT_H
#include <assert.h>

#define G_ASSERT(condition, msg) assert(condition)

#ifndef NDEBUG
#include <stdio.h>
#define debug_print(fmt, ...) \
    do { fprintf(stderr, "%s:%d:%s: " fmt, __FILE__, \
            __LINE__, __func__, __VA_ARGS__); } while (0)
#else
#define debug_print(fmt, ...)
#endif

#endif

