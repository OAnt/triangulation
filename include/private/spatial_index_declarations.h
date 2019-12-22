#ifndef GEOMETRY_SPATIAL_INDEX_DECL_H
#define GEOMETRY_SPATIAL_INDEX_DECL_H

#include <stdlib.h>

/*
 * Structure representing a position in an integer grid.
 */
struct cell_index_st{
    int32_t x; /** X position of the cell. */
    int32_t y; /** Y position of the cell. */
};

/** 
 * Key representing the position of a object in the index.
 */
struct spatial_index_key_st {
    uint32_t level; /** Level were the object reside. 0 means the
                     grid with the coarsest cell size for a given
                     index. */
    struct cell_index_st cell; /** Position in the grid at level. */
};

#define INVALID_LEVEL (uint32_t)-1

/**
 * Provide the caller with a key that represents the box the caller
 * is inserting in the grid.
 * param key Key that represents the object.
 * param data Caller supplied pointer.
 * return ec_no_error on success, returning anything will abort
 * current iteration.
 */
typedef enum error_code_e (*spatial_index_key_computation_callback_f)(
        struct spatial_index_key_st key,
        void * data);

#endif
