#include <stdlib.h>
#include <string.h>
#include <private/common.h>
#include <private/array.h>

static struct array_st _empty_array = {NULL, 0, 0, 0};

void _array_init(
        struct array_st * ar,
        size_t type_size)
{
    *ar = _empty_array;
    ar->type_size = type_size;
}

size_t array_size(
        struct array_st * ar)
{
    return ar->n_elem;
}

#define array_mem_size(ar, n) (ar)->type_size * (n)

#define array_offset(ar, n) (ar)->ptr + array_mem_size(ar, n)

static inline enum error_code_e _array_realloc(
        struct array_st * ar,
        size_t new_n_elem)
{
    if(new_n_elem > ar->m_elem){
        size_t new_m_elem = 2*ar->m_elem;
        new_m_elem = MAX(new_n_elem, new_m_elem);
        char * new_ptr = realloc(ar->ptr,
                array_mem_size(ar, new_m_elem));
        if(!new_ptr) return ec_memory_error;
        ar->ptr = new_ptr;
        ar->m_elem = new_m_elem;
    }
    return ec_no_error;
}

#define array_realloc(ar, new_n_elem) do{ \
    enum error_code_e err = _array_realloc(ar, new_n_elem);\
    if(err != ec_no_error) return ec_no_error;}while(0)

enum error_code_e array_extend(
        struct array_st * ar,
        void * ptr,
        size_t n_elem)
{
    size_t new_n_elem = ar->n_elem + n_elem;
    array_realloc(ar, new_n_elem);
    memmove(array_offset(ar, ar->n_elem),
            ptr, array_mem_size(ar, n_elem));
    ar->n_elem = new_n_elem;
    return ec_no_error;
}

enum error_code_e array_retract(
        struct array_st * ar,
        size_t n_elem,
        void ** ptr,
        size_t * n_removed_elem)
{
    size_t previous_n_elem = ar->n_elem;
    if(ar->n_elem < n_elem){
        ar->n_elem = 0;
    }else{
        ar->n_elem -= n_elem;
    }
    size_t _n_removed_elem = previous_n_elem - ar->n_elem;
    if(n_removed_elem)
        *n_removed_elem = _n_removed_elem;
    if(ptr){
        *ptr = array_offset(ar, ar->n_elem);
    }
    return ec_no_error;
}

enum error_code_e array_set(
        struct array_st * ar,
        void * ptr,
        size_t index,
        size_t n_elem)
{
    size_t new_n_elem = index + n_elem;
    new_n_elem = MAX(new_n_elem, ar->n_elem);
    array_realloc(ar, new_n_elem);
    memmove(array_offset(ar, index),
                ptr, array_mem_size(ar, n_elem));
    ar->n_elem = new_n_elem;
    return ec_no_error;
}

void array_cleanup(
        struct array_st * ar)
{
    if(ar->ptr)
        free(ar->ptr);
    *ar = _empty_array;
}

