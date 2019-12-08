#include <stdlib.h>
#include <string.h>
#include <private/common.h>
#include <private/array.h>

/** 
 * header containing allocation information about
 * an array, it is allocated at the same time as
 * the pointer and is positioned before the address
 * that is returned to caller.
 * |n_elem|m_elem|type_size|array returned to the caller|
 */
struct array_header_st {
    size_t n_elem; /** size requested by the client */
    size_t m_elem; /** size of the underlying array */
    size_t type_size; /** size of the type supposed to be stored in the array */
};

/** compute the starting address for the header of a caller supplied pointer */
#define ARRAY_HEADER(ptr) (((struct array_header_st*)(ptr)) - 1);
/** compute the starting address for the array from a know header */
#define ARRAY_PTR(ptr) (((struct array_header_st*)(ptr)) + 1);
#define ARRAY_SIZE(n, type) (n)*(type) + \
    sizeof(struct array_header_st)

enum error_code_e array_new_(
        size_t type,
        size_t n_elem,
        void ** ptr)
{
    //allocating header + requested size
    struct array_header_st * header = malloc(
            ARRAY_SIZE(n_elem, type));
    if(!header) return ec_memory_error;
    header->m_elem = n_elem;
    header->n_elem = n_elem;
    header->type_size = type;
    //returning a pointer to an array of the
    //requested size
    *ptr = ARRAY_PTR(header);
    return ec_no_error;
}

void array_debug_header(void * ptr, 
        size_t * n,
        size_t * m,
        size_t * size,
        size_t * offset)
{
    //converting the caller supplied pointer to the array header
    struct array_header_st * header = ARRAY_HEADER(ptr);
    *n = header->n_elem;
    *m = header->m_elem;
    *size = header->type_size;
    *offset = (char*)ptr - (char*)header;
}

size_t array_length(void * ptr){
    //converting the caller supplied pointer to the array header
    struct array_header_st * header = ARRAY_HEADER(ptr);
    return header->n_elem;
}

enum error_code_e array_resize_(
        void ** ptr,
        size_t n_elem)
{
    //converting the caller supplied pointer to the array header
    struct array_header_st * header = ARRAY_HEADER(*ptr);
    //No enough memory was allocated, reallocating
    if(n_elem > header->m_elem){
        // choosing a size that at least fits
        size_t new_m_elem = 2*header->m_elem;
        new_m_elem = MAX(n_elem, new_m_elem);
        //allocating header + requested size
        struct array_header_st * new_header = realloc(
                header, ARRAY_SIZE(new_m_elem, header->type_size));
        //something went wrong, not changing and reporting
        if(!new_header) return ec_memory_error;
        header = new_header;
        header->m_elem = new_m_elem;
        //returning a pointer to an array of the
        //requested size
        *ptr = ARRAY_PTR(header)
    }
    header->n_elem = n_elem;
    return ec_no_error;
}

enum error_code_e array_shrink_(
        void ** ptr)
{
    //converting the caller supplied pointer to the array header
    struct array_header_st * header = ARRAY_HEADER(*ptr);
    struct array_header_st * new_header = realloc(
            header, ARRAY_SIZE(header->n_elem, header->type_size));
    if(!new_header) return ec_memory_error;
    new_header->m_elem = new_header->n_elem;
    *ptr = ARRAY_PTR(new_header);
    return ec_no_error;
}

void array_delete_(
        void ** ptr)
{
    //converting the caller supplied pointer to the array header
    struct array_header_st * header = ARRAY_HEADER(*ptr);
    free(header);
    *ptr = NULL;
}

