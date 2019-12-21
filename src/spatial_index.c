#include <math.h>
#include <stdbool.h>
#include <private/debug.h>
#include <private/spatial_index.h>
#include <stdint.h>

/**
 * Root structure for a 2D spatial index.
 */
struct spatial_index_st {
    struct box_st soft_boundaries; /** Boundaries of the index, an object
                                     outside of the can still be inserted
                                     thus the "soft" in the name */
};

/**
 * Computes the level at which box should be inserted. The higher
 * the level, the finer the grid.
 * param soft_boundaries Max size used to determine level. A box that
 * does not fit in soft_boundaries will be at level 0.
 * param box Box for which the level will be computed.
 * return The level at which the box should be inserted.
 */
uint32_t spatial_index_compute_level(
        struct box_st soft_boundaries,
        struct box_st box)
{
    double x_max_cell_size = box_size_along(soft_boundaries, 0);
    double y_max_cell_size = box_size_along(soft_boundaries, 1);
    double x_cell_size = box_size_along(box, 0);
    double y_cell_size = box_size_along(box, 1);
    G_ASSERT(x_max_cell_size > 0 && y_max_cell_size > 0,
            "Invalid index size");
    G_ASSERT(x_cell_size && y_cell_size,
            "Invalid box size");
    uint32_t level = 0;
    uint32_t previous_n_cells = UINT32_MAX;
    while(true){
        uint32_t factor = 1 << level;
        double x_max_cell_size_at_level = x_max_cell_size / factor;
        double y_max_cell_size_at_level = y_max_cell_size / factor;
        uint32_t n_x_cells = (uint32_t)ceil(x_cell_size / x_max_cell_size_at_level); 
        uint32_t n_y_cells = (uint32_t)ceil(y_cell_size / y_max_cell_size_at_level);
        uint32_t n_cells = n_x_cells * n_y_cells;
        debug_print("%d, %d\n", n_cells, previous_n_cells);
        // The box is bigger than the cell stop here (level = 1)
        // The correct level is at an inflexion point
        if(n_cells > previous_n_cells){
            return level- 1;
        }else{
            previous_n_cells = n_cells;
            level += 1;
        }
    }
}

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
    int32_t level; /** Level were the object reside. 0 means the
                     grid with the coarsest cell size for a given
                     index. */
    struct cell_index_st cell; /** Position in the grid at level. */
};

/**
 * Provide the caller with a key that represents the box the caller
 * is inserting in the grid.
 * param key Key that represents the object.
 * param data Caller supplied pointer.
 */
typedef void (*spatial_index_key_computation_callback_f)(
        struct spatial_index_key_st key,
        void * data);

/**
 * Computes the eventual list of cell keys that represents a given box.
 * param spi Spatial index inside which the box will be ultimately inserted.
 * param box Box for which the keys will be computed.
 * param callback Caller supplied function that will be called to forward
 * any eventual key that will be found.
 * param data Caller supplied pointer forwarded to the callback.
 */
void spatial_index_compute_keys(
        struct spatial_index_st * spi,
        struct box_st box,
        spatial_index_key_computation_callback_f callback,
        void * data)
{
    
}
