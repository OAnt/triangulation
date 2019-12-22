#include <float.h>
#include <math.h>
#include <private/array.h>
#include <private/debug.h>
#include <private/grid_register.h>
#include <private/spatial_hash.h>

/**
 * Object representing the spatial hash
 */
struct spatial_hash_st {
    struct grid_register_list_st grid_registers; /** Storage of registers. */
    size_t * buckets; /** List of buckets, index of the first
                        register in the bucket. */
    int32_t n_x_bkts; /** Number of buckets along x */
    int32_t n_y_bkts; /** Number of buckets along y */
    double x_cell_size; /** Size of a bucket along x */
    double y_cell_size; /** Size of a bucket along y */
    uint32_t n_levels; /** Number of levels in the hierarchical grid */
    size_t * levels;/** Number of register at each level */
};

enum error_code_e spatial_hash_init(
        struct spatial_hash_st * sph,
        int32_t n_x_bkts,
        int32_t n_y_bkts,
        double x_cell_size,
        double y_cell_size)
{
    enum error_code_e err = grid_register_list_init(&sph->grid_registers);
    if(err != ec_no_error) goto fail_no_reg;
    err = array_new(size_t, n_x_bkts * n_y_bkts, &sph->buckets);
    for(size_t i = 0; i < array_length(sph->buckets); i++){
        sph->buckets[i] = INVALID_INDEX;
    }
    if(err != ec_no_error) goto fail_no_bkt;
    sph->n_x_bkts = n_x_bkts;
    sph->n_y_bkts = n_y_bkts;
    sph->x_cell_size = x_cell_size;
    sph->y_cell_size = y_cell_size;
    double n_bkts = (double)MAX(n_x_bkts, n_y_bkts);
    sph->n_levels = (uint32_t)ceil(log(n_bkts) / log(2));
    sph->levels = calloc(sph->n_levels, sizeof(size_t));
    if(sph->levels == NULL) goto fail_no_levels;
    return err;
fail_no_levels:
    array_delete(&sph->buckets);
fail_no_bkt:
    grid_register_list_cleanup(&sph->grid_registers);
fail_no_reg:
    return err;
}

enum error_code_e spatial_hash_new(
        int32_t n_x_bkts,
        int32_t n_y_bkts,
        double x_cell_size,
        double y_cell_size,
        struct spatial_hash_st ** _sph)
{
    struct spatial_hash_st * sph = calloc(1, sizeof(struct spatial_hash_st));
    if(sph == NULL) return ec_memory_error;
    enum error_code_e err = spatial_hash_init(
            sph, n_x_bkts, n_y_bkts, x_cell_size, y_cell_size);
    if(err != ec_no_error){
        free(sph);
        return err;
    }else{
        *_sph = sph;
        return ec_no_error;
    }
}

void spatial_hash_cleanup(
        struct spatial_hash_st * sph)
{
    array_delete(&sph->buckets);
    grid_register_list_cleanup(&sph->grid_registers);
    free(sph->levels);
    memset(sph, 0, sizeof(struct spatial_hash_st));
}

void spatial_hash_delete(
        struct spatial_hash_st ** sph)
{
    spatial_hash_cleanup(*sph);
    free(*sph);
    *sph = NULL;
}

typedef enum error_code_e (*spatial_hash_bucket_iterator_callback_f)(
        struct spatial_hash_st * sph,
        struct spatial_index_key_st key,
        void * data);

static inline enum error_code_e spatial_hash_iterate_over_buckets(
        struct spatial_hash_st * sph,
        struct vector_st min,
        struct vector_st max,
        uint32_t _level,
        spatial_hash_bucket_iterator_callback_f iterator_callback,
        void * data)
{
    G_ASSERT(max.v[0] >= min.v[0], "Max must be greater or equal than min");
    G_ASSERT(max.v[1] >= min.v[1], "Max must be greater or equal than min");
    // Computing the span of the intersection between the object
    // bounded by min and max with the spatial hash
    // (a grid of x_cell_size x y_cell_size cells)
    // Objects that are out of the grid bounds can be handled (the % ensures
    // that the grid is infinite)
    uint32_t level = 1 << _level;
    double x_cell_size = sph->x_cell_size * level;
    double y_cell_size = sph->y_cell_size * level;
    int32_t orig_x = ((int32_t)trunc(min.v[0] / x_cell_size) % sph->n_x_bkts);
    if(orig_x < 0) orig_x += sph->n_x_bkts;
    int32_t orig_y = ((int32_t)trunc(min.v[1] / y_cell_size) % sph->n_y_bkts);
    if(orig_y < 0) orig_y += sph->n_y_bkts;
    int32_t end_x = ((int32_t)ceil(max.v[0] / x_cell_size) % sph->n_x_bkts);
    if(end_x < 0) end_x += sph->n_x_bkts;
    int32_t end_y = ((int32_t)ceil(max.v[1] / y_cell_size) % sph->n_y_bkts);
    if(end_y < 0) end_y += sph->n_y_bkts;
    int32_t x = orig_x;
    do{
        int32_t y = orig_y;
        do{
            struct spatial_index_key_st key = {_level, {x, y}};
            enum error_code_e err = iterator_callback(sph, key, data);
            if(err != ec_no_error) return err;
            y = (y + 1) % sph->n_y_bkts;
        }while(y != end_y);
        x = (x + 1) % sph->n_x_bkts;
    }while(x != end_x);
    return ec_no_error;
}

struct spatial_hash_add_callback_data_st{
    size_t index;
    size_t * handle;
    uint32_t level;
};

#define get_bucket(sph, key) (key).cell.y * (sph)->n_x_bkts + (key).cell.x

enum error_code_e spatial_hash_add_iterator_callback(
        struct spatial_hash_st * sph,
        struct spatial_index_key_st key,
        void * _data)
{
    struct spatial_hash_add_callback_data_st * data = 
        (struct spatial_hash_add_callback_data_st *)_data;
    size_t reg_index;
    // A new register is needed, if it cannot be allocated stop
    // a forward the error to the caller
    enum error_code_e err = grid_register_list_new_register(
            &sph->grid_registers, &reg_index);
    if(err != ec_no_error) return err;
    struct grid_register_st * reg = &sph->grid_registers.registers[reg_index];
    // Storing user supplied index
    reg->index = data->index;
    size_t bucket = get_bucket(sph, key);
    grid_register_prepend_to_lists(
            &sph->grid_registers, reg_index, sph->buckets[bucket], data->handle);
    reg->key = key;
    sph->buckets[bucket] = reg_index;
    sph->levels[data->level] += 1;
    return ec_no_error;
}

uint32_t spatial_hash_find_level(
        struct spatial_hash_st * sph,
        struct vector_st min,
        struct vector_st max)
{
    double x_size = max.v[0] - min.v[0];
    double y_size = max.v[1] - min.v[1];
    double previous_n_cells = DBL_MAX;
    uint32_t previous_level = 0;
    uint32_t level = 0;
    while(level < sph->n_levels){
        double n_x_cells = ceil(x_size / (sph->x_cell_size * (1 << level)));
        double n_y_cells = ceil(y_size / (sph->y_cell_size * (1 << level)));
        double n_cells = n_x_cells * n_y_cells;
        if(n_cells == 1.0 || n_cells == previous_n_cells){
            return previous_level;
        }else{
            previous_level = level;
            previous_n_cells = n_cells;
            level += 1;
        }
    }
    return level - 1;
}

enum error_code_e spatial_hash_add(
        struct spatial_hash_st * sph,
        struct vector_st min,
        struct vector_st max,
        size_t index,
        size_t * handle)
{
    G_ASSERT(handle != NULL,
            "This function needs a valid pointer to an handle");
    *handle = INVALID_INDEX;
    uint32_t level = spatial_hash_find_level(sph, min, max);
    G_ASSERT(level < sph->n_levels,
            "The computed level is out of bounds");
    struct spatial_hash_add_callback_data_st data = {index, handle, level};
    return spatial_hash_iterate_over_buckets(
            sph, min, max, level, spatial_hash_add_iterator_callback, &data);
}

struct spatial_hash_get_callback_data_st{
    spatial_hash_get_callback_f get_callback;
    void * data;
    uint32_t level;
};

enum error_code_e spatial_hash_get_iterator_callback(
        struct spatial_hash_st * sph,
        struct spatial_index_key_st key,
        void * _data)
{
    size_t bucket_index = get_bucket(sph, key);
    struct spatial_hash_get_callback_data_st * data =
        (struct spatial_hash_get_callback_data_st *)_data;
    G_ASSERT(bucket_index < array_length(sph->buckets),
            "Bucket is out of bounds");
    // The queried box intersects the bucket pointed
    // by bucket_index, iterating over its list of
    // register, for each one, notify the caller that
    // there is something in there by forwarding the 
    // register index (which was supplied in the first
    // place)
    size_t next_bkt_reg = sph->buckets[bucket_index];
    while(next_bkt_reg != INVALID_INDEX){
        if(sph->grid_registers.registers[next_bkt_reg].key.level != data->level){
            next_bkt_reg = sph->grid_registers.registers[next_bkt_reg].next_bkt_reg;
            G_ASSERT(next_bkt_reg != sph->buckets[bucket_index],
                    "Loop detected");
            continue;
        }
        G_ASSERT(sph->grid_registers.registers[next_bkt_reg].key.level != INVALID_LEVEL,
                "Unset register in linked list");
        bool stop = data->get_callback(
                sph->grid_registers.registers[next_bkt_reg].index,
                data->data);
        // The caller asked to stop, complying
        if(stop) return ec_error;
        next_bkt_reg = sph->grid_registers.registers[next_bkt_reg].next_bkt_reg;
        G_ASSERT(next_bkt_reg != sph->buckets[bucket_index],
                "Loop detected");
    }
    return ec_no_error;
}

void spatial_hash_get(
        struct spatial_hash_st * sph,
        struct vector_st min,
        struct vector_st max,
        spatial_hash_get_callback_f get_callback,
        void * data)
{
    struct spatial_hash_get_callback_data_st get_data = {
        get_callback, data, 0};
    for(uint32_t l = 0; l < sph->n_levels; l++){
        if(!sph->levels[l]) continue;
        get_data.level = l;
        enum error_code_e err = spatial_hash_iterate_over_buckets(
                sph, min, max, l, spatial_hash_get_iterator_callback, &get_data);
        // caller asked for a stop
        if(err != ec_no_error) return;
    }
}

void spatial_hash_remove_callback(
        struct grid_register_st * reg,
        void * data)
{
    struct spatial_hash_st * sph = (struct spatial_hash_st*)data;
    size_t bucket = get_bucket(sph, reg->key);
    // This is the first in the list, it is pointed
    // by the bucket index, if it is being removed
    // the bucket must point to something valid
    if(reg->prev_bkt_reg == INVALID_INDEX){
        sph->buckets[bucket] = reg->next_bkt_reg;
    }
    G_ASSERT(sph->levels[reg->key.level] > 0,
            "There is already no objects at this level");
    sph->levels[reg->key.level] -= 1;
}

void spatial_hash_remove(
        struct spatial_hash_st * sph,
        size_t handle)
{
    grid_register_list_remove_registers_by_handle(
            &sph->grid_registers,
            handle,
            spatial_hash_remove_callback,
            sph);
}

