#ifndef GEOMETRY_ARRAY_H
#define GEOMETRY_ARRAY_H

#include <string.h>
#include <private/common.h>

enum error_code_e array_new_(
        size_t type,
        size_t n_elem,
        void ** ptr);

#define array_new(type, n_elem, ptr) array_new_(sizeof(type), (n_elem), (void **)(ptr))

size_t array_length(void * ptr);

enum error_code_e array_resize_(
        void ** ptr,
        size_t n_elem);

#define array_resize(ptr, n_elem) array_resize_((void**)(ptr), (n_elem))

enum error_code_e array_delete_(
        void ** ptr);

#define array_delete(ptr) array_delete_((void**)(ptr))

enum error_code_e array_shrink_(
        void ** ptr);

#define array_shrink(ptr) array_shrink_((void**)(ptr))

#endif
