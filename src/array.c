#include <stdlib.h>
#include <string.h>
#include <private/common.h>
#include <private/array.h>

struct array_header_st {
    size_t n_elem;
    size_t m_elem;
    size_t type_size;
};

#define ARRAY_HEADER(ptr) (((struct array_header_st*)(ptr)) - 1);
#define ARRAY_PTR(ptr) (((struct array_header_st*)(ptr)) + 1);
#define ARRAY_SIZE(n, type) (n)*(type) + \
    sizeof(struct array_header_st) + sizeof(size_t)

enum error_code_e array_new_(
        size_t type,
        size_t n_elem,
        void ** ptr)
{
    struct array_header_st * header = malloc(
            ARRAY_SIZE(n_elem, type));
    if(!header) return ec_memory_error;
    header->m_elem = n_elem;
    header->n_elem = n_elem;
    header->type_size = type;
    *ptr = ARRAY_PTR(header);
    return ec_no_error;
}

void array_debug_header(void * ptr, 
        size_t * n,
        size_t * m,
        size_t * size,
        size_t * offset)
{
    struct array_header_st * header = ARRAY_HEADER(ptr);
    *n = header->n_elem;
    *m = header->m_elem;
    *size = header->type_size;
    *offset = (char*)ptr - (char*)header;
}

size_t array_length(void * ptr){
    struct array_header_st * header = ARRAY_HEADER(ptr);
    return header->n_elem;
}

enum error_code_e array_resize_(
        void ** ptr,
        size_t n_elem)
{
    struct array_header_st * header = ARRAY_HEADER(*ptr);
    if(n_elem > header->m_elem){
        size_t new_m_elem = 2*header->m_elem;
        new_m_elem = MAX(n_elem, new_m_elem);
        struct array_header_st * new_header = realloc(
                header, ARRAY_SIZE(new_m_elem, header->type_size));
        if(!new_header) return ec_memory_error;
        header = new_header;
        header->m_elem = new_m_elem;
        *ptr = ARRAY_PTR(header)
    }
    header->n_elem = n_elem;
    return ec_no_error;
}

enum error_code_e array_shrink_(
        void ** ptr)
{
    struct array_header_st * header = ARRAY_HEADER(*ptr);
    struct array_header_st * new_header = realloc(
            header, ARRAY_SIZE(header->n_elem, header->type_size));
    if(!new_header) return ec_memory_error;
    new_header->m_elem = new_header->n_elem;
    *ptr = ARRAY_PTR(new_header);
    return ec_no_error;
}

enum error_code_e array_delete_(
        void ** ptr)
{
    struct array_header_st * header = ARRAY_HEADER(*ptr);
    free(header);
    *ptr = NULL;
    return ec_no_error;
}

