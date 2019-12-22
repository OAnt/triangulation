#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include <m-dict.h>

#include <private/array.h>
#include <private/debug.h>
#include <private/grid_register.h>
#include <private/spatial_index.h>
#include <private/spatial_index_declarations.h>

DICT_DEF2(spatial_index_grid, struct spatial_index_key_st, M_POD_OPLIST, size_t, M_DEFAULT_OPLIST)

/**
 * Root structure for a 2D spatial index.
 */
struct spatial_index_st {
    spatial_index_grid_t grid; /** Dictionary representing the 
                                 index grid, this way it can grow as
                                 needed (especially down) */
    struct grid_register_list_st grs; /** Lists of objects
                                        that have been added to the index */
    int32_t * levels;
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
    if(grid_register_list_init(&(*spi)->grs) == ec_memory_error){
        free(*spi);
        return ec_memory_error;
    }
    if(array_new(int32_t, 0, &(*spi)->levels) == ec_memory_error){
        free(*spi);
        grid_register_list_cleanup(&(*spi)->grs);
        return ec_memory_error;
    }
    spatial_index_grid_init((*spi)->grid);
    (*spi)->soft_boundaries = soft_boundaries;
    return ec_no_error;
}

void spatial_index_delete(
        struct spatial_index_st * spi)
{
    spatial_index_grid_clean(spi->grid);
    grid_register_list_cleanup(&spi->grs);
    array_delete(&spi->levels);
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

struct spatial_index_add_data_st{
    struct spatial_index_st * spi;
    size_t * handle;
    size_t index;
};

static inline size_t get_bucket(
        struct spatial_index_st * spi,
        struct spatial_index_key_st key)
{
    size_t * bucket_ptr = spatial_index_grid_get(spi->grid, key);
    size_t bucket;
    // The grid cell does not exist, reg_index is its first element,
    // By providing INVALID_INDEX subsequent functions will
    // understand that this bucket is empty
    if(bucket_ptr == NULL){
        bucket = INVALID_INDEX;    
    }else{
    // Grid cell exits, simply adding another element
        bucket = *bucket_ptr;
    }
    return bucket;
}

static enum error_code_e spatial_index_add_iterator_callback(
        struct spatial_index_key_st key,
        void * _data)
{
    struct spatial_index_add_data_st * data = 
        (struct spatial_index_add_data_st *)_data;
    struct spatial_index_st * spi = data->spi;
    size_t reg_index;
    // A new register is needed, if it cannot be allocated stop
    // a forward the error to the caller
    enum error_code_e err = grid_register_list_new_register(
            &spi->grs, &reg_index);
    if(err != ec_no_error) return err;
    struct grid_register_st * reg = &spi->grs.registers[reg_index];
    // Storing user supplied index
    reg->index = data->index;
    reg->key = key;
    // Increment the count of objects at level
    spi->levels[key.level] += 1;
    // probing the grid to see if the grid cell corresponding to key
    // exists
    size_t bucket = get_bucket(spi, key);
    grid_register_prepend_to_lists(
            &spi->grs, reg_index, bucket, data->handle);
    // Updating the grid to point at the new head;
    spatial_index_grid_set_at(spi->grid, key, reg_index);
    /*debug_print("index %ld registered at <%d, <%d, %d> (%ld)>\n",*/
            /*reg->index, key.level, key.cell.x, key.cell.y, reg_index);*/
    return ec_no_error;
}

enum error_code_e spatial_index_add(
        struct spatial_index_st * spi,
        struct box_st box,
        size_t index,
        size_t * handle)
{
    G_ASSERT(handle != NULL,
            "This function needs a valid pointer to an handle");
    *handle = INVALID_INDEX;
    uint32_t level = spatial_index_compute_level(
            spi->soft_boundaries,
            box);
    size_t n_levels = array_length(spi->levels);
    // there are new levels (an object at least 2 x smaller than everything
    // that was inserted beforehand) is being inserted
    // Adding levels and zeroing because nothing is there yet
    if(level >= n_levels){
        enum error_code_e err = array_resize(&spi->levels, level + 1);
        if(err != ec_no_error) return err;
        memset(
                spi->levels + n_levels,
                0,
                (level - n_levels + 1) * sizeof(uint32_t));
    }
    struct spatial_index_add_data_st data = {spi, handle, index};
    return spatial_index_locate_on_grid(
            spi->soft_boundaries, 
            box,
            level,
            spatial_index_add_iterator_callback,
            &data);
}

struct spatial_index_get_data_st{
    struct spatial_index_st * spi;
    spatial_index_get_callback_f get_callback;
    void * data; 
};

static enum error_code_e spatial_index_get_iterator_callback(
        struct spatial_index_key_st key,
        void * _data)
{
    struct spatial_index_get_data_st * data = 
        (struct spatial_index_get_data_st *) _data;
    struct spatial_index_st * spi = data->spi;
    size_t bucket = get_bucket(spi, key);
    size_t next_bkt_reg = bucket;
    // The queried box intersects the bucket pointed
    // by bucket, iterating over its list of
    // register, for each one, notify the caller that
    // there is something in there by forwarding the 
    // register index (which was supplied in the first
    // place)
    while(next_bkt_reg != INVALID_INDEX){
        G_ASSERT(spi->grs.registers[next_bkt_reg].key.level != INVALID_LEVEL,
                "Unset register in linked list");
        /*struct grid_register_st * reg = &spi->grs.registers[next_bkt_reg];*/
        /*debug_print("index %ld got from <%d, <%d, %d>> (%ld)\n",*/
                /*reg->index, reg->key.level, reg->key.cell.x, reg->key.cell.y, next_bkt_reg);*/
        bool stop = data->get_callback(
                spi->grs.registers[next_bkt_reg].index,
                data->data);
        // The caller asked to stop, complying
        if(stop) return ec_error;
        next_bkt_reg = spi->grs.registers[next_bkt_reg].next_bkt_reg;
        G_ASSERT(next_bkt_reg != bucket,
                "Loop detected");
    }
    return ec_no_error;
}

void spatial_index_get(
        struct spatial_index_st * spi,
        struct box_st box,
        spatial_index_get_callback_f get_callback,
        void * data)
{
    struct spatial_index_get_data_st get_data = {
        spi, get_callback, data};
    size_t n_levels = array_length(spi->levels);
    for(uint32_t i = 0; i < n_levels; i++){
        if(!spi->levels[i]) continue;
        enum error_code_e err = spatial_index_locate_on_grid(
                spi->soft_boundaries,
                box,
                i,
                spatial_index_get_iterator_callback,
                &get_data);
        // caller asked for a stop
        if(err != ec_no_error) return;
    }
}

void spatial_index_remove_callback(
        struct grid_register_st * reg,
        void * data)
{
    struct spatial_index_st * spi = 
        (struct spatial_index_st *)data;
    G_ASSERT(spi->levels[reg->key.level] > 0,
            "There is already no objects at this level");
    spi->levels[reg->key.level] -= 1;
    // This is the first in the list, it is pointed
    // by the bucket index, if it is being removed
    // the bucket must point to something valid
    if(reg->prev_bkt_reg == INVALID_INDEX){
        if(reg->next_bkt_reg == INVALID_INDEX){
            spatial_index_grid_erase(spi->grid, reg->key);
        }else{
            spatial_index_grid_set_at(spi->grid, reg->key, reg->next_bkt_reg);
        }
    }
    /*debug_print("index %ld unregistered from <%d, <%d, %d>> (%ld)\n",*/
            /*reg->index, reg->key.level, reg->key.cell.x, reg->key.cell.y,*/
            /*reg - spi->grs.registers);*/
}

void spatial_index_remove(
        struct spatial_index_st * spi,
        size_t handle)
{
    grid_register_list_remove_registers_by_handle(
            &spi->grs,
            handle,
            spatial_index_remove_callback,
            spi);
}

