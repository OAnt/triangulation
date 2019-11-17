#ifndef GEOMETRY_ARRAY_H
#define GEOMETRY_ARRAY_H

#include <string.h>
#include <private/common.h>

/**
 * Allocates a continuous memory chunk able to contain n_elem
 * of size type.
 * param type Size of elements in the array.
 * param n_elem Maximum number of element in the array.
 * param ptr The pointer to the allocated chunk
 * return ec_no_error if the function succeeds
 */
enum error_code_e array_new_(
        size_t type,
        size_t n_elem,
        void ** ptr);

/**
 * Convenience macro transforms the type to its size
 */
#define array_new(type, n_elem, ptr) array_new_(sizeof(type), (n_elem), (void **)(ptr))

/**
 * Returns the maximum number of elements.
 * param ptr Continuous memory chunk.
 */
size_t array_length(void * ptr);

/**
 * Resizes the array to that it can fit up to n_elem
 * param ptr Reference to the array to resize, ptr maybe modified
 * by reallocation
 * param n_elem New size for the array
 * return ec_no_error if the function succeeds
 */
enum error_code_e array_resize_(
        void ** ptr,
        size_t n_elem);

/**
 * Convenience macro to avoid having to recast the pointer
 */
#define array_resize(ptr, n_elem) array_resize_((void**)(ptr), (n_elem))

/**
 * Frees the array referenced by pointer and sets it to NULL
 * param ptr Reference to the array to free
 * return ec_no_error if the function succeeds
 */
enum error_code_e array_delete_(
        void ** ptr);

/**
 * Convinience macro to avoid having to recast the pointer
 */
#define array_delete(ptr) array_delete_((void**)(ptr))

/**
 * Resizes pointer so that it takes the minimum amount of memory
 * possible to store array_length(*ptr) elements
 * param ptr Reference to the array to resize.
 * return ec_no_error if the function succeeds
 */
enum error_code_e array_shrink_(
        void ** ptr);

/**
 * Convenience macro transforms the type to its size
 */
#define array_shrink(ptr) array_shrink_((void**)(ptr))

#endif
