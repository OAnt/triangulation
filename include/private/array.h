#ifndef GEOMETRY_ARRAY_H
#define GEOMETRY_ARRAY_H

#include <string.h>
#include <private/common.h>

struct array_st {
    char * ptr;
    size_t n_elem;
    size_t m_elem;
    size_t type_size;
};

void _array_init(
        struct array_st * ar,
        size_t type_size);

#define array_init(array, type) _array_init((array), (sizeof(type)))

#define array_ptr(array) (array)->ptr

size_t array_size(
        struct array_st * ar);

enum error_code_e array_extend(
        struct array_st * ar,
        void * ptr,
        size_t n_elem);

enum error_code_e array_retract(
        struct array_st * ar,
        size_t n_elem,
        void ** ptr,
        size_t * n_removed_elem);

enum error_code_e array_set(
        struct array_st * ar,
        void * ptr,
        size_t index,
        size_t n_elem);

void array_cleanup(
        struct array_st * ar);

#endif
