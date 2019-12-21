#include <math.h>
#include <stdbool.h>
#include <private/debug.h>
#include <private/spatial_index.h>
#include <private/spatial_index_declarations.h>
#include <stdint.h>

/**
 * Root structure for a 2D spatial index.
 */
struct spatial_index_st {
    struct box_st soft_boundaries; /** Boundaries of the index, an object
                                     outside of the can still be inserted
                                     thus the "soft" in the name */
};

enum error_code_e spatial_index_new(
        struct box_st soft_boundaries,
        struct spatial_index_st ** spi)
{
    *spi = calloc(1, sizeof(struct spatial_index_st));
    if(!*spi) return ec_memory_error;
    (*spi)->soft_boundaries = soft_boundaries;
    return ec_no_error;
}

void spatial_index_delete(
        struct spatial_index_st * spi)
{
    free(spi);
}

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

/**
 * Computes the eventual list of cell keys that represents a given box.
 * param spi Spatial index inside which the box will be ultimately inserted.
 * param box Box for which the keys will be computed.
 * param callback Caller supplied function that will be called to forward
 * any eventual key that will be found.
 * param level The grid level at which the computation takes place.
 * param data Caller supplied pointer forwarded to the callback.
 * return ec_no_error on success, forwards error on caller abort.
 */
enum error_code_e spatial_index_locate_on_grid(
        struct box_st soft_boundaries,
        struct box_st _box,
        uint32_t level,
        spatial_index_key_computation_callback_f callback,
        void * data)
{
    struct box_st box;
    vector_subtraction(&_box.min, &soft_boundaries.min, &box.min);
    vector_subtraction(&_box.max, &soft_boundaries.min, &box.max);
    G_ASSERT(box.max.v[0] >= box.min.v[0],
            "Max must be greater or equal than min");
    G_ASSERT(box.max.v[1] >= box.min.v[1],
            "Max must be greater or equal than min");
    uint32_t factor = 1 << level;
    double x_cell_size = (box_size_along(soft_boundaries, 0)) / factor;
    double y_cell_size = (box_size_along(soft_boundaries, 1)) / factor;
    int32_t orig_x = (int32_t)floor(box.min.v[0] / x_cell_size);
    int32_t orig_y = (int32_t)floor(box.min.v[1] / y_cell_size);
    int32_t end_x = (int32_t)ceil(box.max.v[0] / x_cell_size);
    int32_t end_y = (int32_t)ceil(box.max.v[1] / y_cell_size);
    for(int32_t x = orig_x; x < end_x; x++){
        for(int32_t y = orig_y; y < end_y; y++){
            struct spatial_index_key_st key = {level, {x, y}};
            enum error_code_e err = callback(key, data);
            if(err != ec_no_error) return err;
        }
    }
    return ec_no_error;
}

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
    uint32_t level = spatial_index_compute_level(
            spi->soft_boundaries,
            box);
    spatial_index_locate_on_grid(
            spi->soft_boundaries, box, level, callback, data);
}
