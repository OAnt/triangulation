#include "public/common.h"
#include <math.h>
#include <private/array.h>
#include <private/debug.h>
#include <private/spatial_hash.h>

/**
 * Member of the linked list of objects stored in
 * a bucket.
 */
struct spatial_hash_register_st{
    size_t index; /** Index of the stored object. */
    size_t next_obj_reg; /** Index of the next register
                           for the inserted object if it
                           spans over multiple bucket. */
    size_t next_bkt_reg; /** Index of the next register
                           in the bucket. */
    size_t prev_bkt_reg; /** Index of the previous register
                           in the bucket. */
    size_t bucket; /** The register is in this bucket. */
};

/**
 * Object representing the spatial hash
 */
struct spatial_hash_st {
    struct spatial_hash_register_st * registers; /** Storage of registers. */
    size_t * buckets; /** List of buckets, index of the first
                        register in the bucket. */
    int32_t n_x_bkts; /** Number of buckets along x */
    int32_t n_y_bkts; /** Number of buckets along y */
    double x_cell_size; /** Size of a bucket along x */
    double y_cell_size; /** Size of a bucket along y */
    size_t collected_registers; /** Index of the first register in the
                                  linked list of unused registers */
};

enum error_code_e spatial_hash_init(
        struct spatial_hash_st * sph,
        int32_t n_x_bkts,
        int32_t n_y_bkts,
        double x_cell_size,
        double y_cell_size)
{
    enum error_code_e err = array_new(
            struct spatial_hash_register_st, 0, &sph->registers);
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
    sph->collected_registers = INVALID_INDEX;
    return err;
fail_no_bkt:
    array_delete(&sph->registers);
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
    array_delete(&sph->registers);
    memset(sph, 0, sizeof(struct spatial_hash_st));
}

void spatial_hash_delete(
        struct spatial_hash_st ** sph)
{
    spatial_hash_cleanup(*sph);
    free(*sph);
    *sph = NULL;
}

void spatial_hash_register_remove_from_list(
        struct spatial_hash_st * sph,
        struct spatial_hash_register_st * reg)
{
    size_t previous_index = reg->prev_bkt_reg;
    size_t next_index = reg->next_bkt_reg;
    if(next_index != INVALID_INDEX){
        sph->registers[next_index].prev_bkt_reg = previous_index;
    }
    if(previous_index != INVALID_INDEX){
        sph->registers[previous_index].next_bkt_reg = next_index;
    }
}

enum error_code_e spatial_hash_new_register(
        struct spatial_hash_st * sph,
        size_t * _reg)
{
    struct spatial_hash_register_st * reg;
    // a register is available (already removed)
    if(sph->collected_registers != INVALID_INDEX){
        reg = &sph->registers[sph->collected_registers];
        G_ASSERT(reg->prev_bkt_reg == INVALID_INDEX &&
                reg->next_obj_reg == INVALID_INDEX &&
                reg->index == INVALID_INDEX &&
                reg->bucket == INVALID_INDEX,
                "A collected element was not uninitialized");
        *_reg = sph->collected_registers;;
        // removing the object from the list is in (list of unused)
        sph->collected_registers = reg->next_bkt_reg;
        spatial_hash_register_remove_from_list(sph, reg);
    // No register available in the list of previously removed
    // allocating
    }else{
        size_t index = array_length(sph->registers);
        enum error_code_e err = array_resize(&sph->registers, index + 1);
        if(err != ec_no_error) return err;
        *_reg = index;
    }
    return ec_no_error;
}

typedef enum error_code_e (*spatial_hash_bucket_iterator_callback_f)(
        struct spatial_hash_st * sph, size_t bucket_index, void * data);

static inline enum error_code_e spatial_hash_iterate_over_buckets(
        struct spatial_hash_st * sph,
        struct vector_st min,
        struct vector_st max,
        spatial_hash_bucket_iterator_callback_f iterator_callback,
        void * data,
        const char * caller)
{
    (void)caller;
    G_ASSERT(max.v[0] >= min.v[0], "Max must be greater or equal than min");
    G_ASSERT(max.v[1] >= min.v[1], "Max must be greater or equal than min");
    // Computing the span of the intersection between the object
    // bounded by min and max with the spatial hash
    // (a grid of x_cell_size x y_cell_size cells)
    // Objects that are out of the grid bounds can be handled (the % ensures
    // that the grid is infinite)
    int32_t orig_x = ((int32_t)trunc(min.v[0] / sph->x_cell_size) % sph->n_x_bkts);
    if(orig_x < 0) orig_x += sph->n_x_bkts;
    int32_t orig_y = ((int32_t)trunc(min.v[1] / sph->y_cell_size) % sph->n_y_bkts);
    if(orig_y < 0) orig_y += sph->n_y_bkts;
    int32_t end_x = ((int32_t)ceil(max.v[0] / sph->x_cell_size) % sph->n_x_bkts);
    if(end_x < 0) end_x += sph->n_x_bkts;
    int32_t end_y = ((int32_t)ceil(max.v[1] / sph->y_cell_size) % sph->n_y_bkts);
    if(end_y < 0) end_y += sph->n_y_bkts;
    int32_t x = orig_x;
    do{
        int32_t y = orig_y;
        do{
            size_t bucket = y * sph->n_x_bkts + x;
            enum error_code_e err = iterator_callback(sph, bucket, data);
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
};

enum error_code_e spatial_hash_add_iterator_callback(
        struct spatial_hash_st * sph,
        size_t bucket,
        void * _data)
{
    struct spatial_hash_add_callback_data_st * data = 
        (struct spatial_hash_add_callback_data_st *)_data;
    size_t reg_index;
    // A new register is needed, if it cannot be allocated stop
    // a forward the error to the caller
    enum error_code_e err = spatial_hash_new_register(sph, &reg_index);
    if(err != ec_no_error) return err;
    struct spatial_hash_register_st * reg = &sph->registers[reg_index];
    // Storing user supplied index
    reg->index = data->index;
    // Adding the object to the one way linked list
    // of objects that corresponds to the index
    reg->next_obj_reg = *data->handle;
    // An handle (head of the list)
    // is returned to the caller in order to allow
    // removal of the object from the spatial hash
    // as the object (index) may be spread over
    // several buckets
    *data->handle = reg_index;
    // Adding (prepending) the register to the bucket that 
    // was found to intersect it, this is a
    // two way list to allow easy removal for the
    // bucket list.
    reg->next_bkt_reg = sph->buckets[bucket];
    reg->prev_bkt_reg = INVALID_INDEX;
    reg->bucket = bucket;
    if(reg->next_bkt_reg != INVALID_INDEX){
        sph->registers[reg->next_bkt_reg].prev_bkt_reg = reg_index;
    }
    sph->buckets[bucket] = reg_index;
    return ec_no_error;
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
    struct spatial_hash_add_callback_data_st data = {index, handle};
    return spatial_hash_iterate_over_buckets(
            sph, min, max, spatial_hash_add_iterator_callback, &data, __func__);
}

struct spatial_hash_get_callback_data_st{
    spatial_hash_get_callback_f get_callback;
    void * data;
};

enum error_code_e spatial_hash_get_iterator_callback(
        struct spatial_hash_st * sph,
        size_t bucket_index,
        void * _data)
{
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
        G_ASSERT(sph->registers[next_bkt_reg].bucket != INVALID_INDEX,
                "Unset register in linked list");
        bool stop = data->get_callback(
                sph->registers[next_bkt_reg].index,
                data->data);
        // The caller asked to stop, complying
        if(stop) return ec_error;
        next_bkt_reg = sph->registers[next_bkt_reg].next_bkt_reg;
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
        get_callback, data};
    spatial_hash_iterate_over_buckets(sph, min, max,
            spatial_hash_get_iterator_callback, &get_data, __func__);
}

void spatial_hash_remove(
        struct spatial_hash_st * sph,
        size_t handle)
{
    G_ASSERT(handle < array_length(sph->registers),
            "Handle is not in the list of registers");
    size_t next_obj_reg = handle;
    while(next_obj_reg != INVALID_INDEX){
        struct spatial_hash_register_st * reg = &sph->registers[next_obj_reg];
        G_ASSERT(reg->bucket != INVALID_INDEX,
                "Handle was already removed");
        spatial_hash_register_remove_from_list(sph, reg);
        size_t collected_list = sph->collected_registers;
        // This is the first in the list, it is pointed
        // by the bucket index, if it is being removed
        // the bucket must point to something valid
        if(reg->prev_bkt_reg == INVALID_INDEX){
            sph->buckets[reg->bucket] = reg->next_bkt_reg;
        }
        // Setting obviously wrong value than can
        // be sanity checked later on
        reg->index = INVALID_INDEX;
        reg->bucket = INVALID_INDEX;
        reg->next_bkt_reg = collected_list;
        reg->prev_bkt_reg = INVALID_INDEX;
        // Adding the register to the list of collected
        // registers
        if(collected_list != INVALID_INDEX){
            sph->registers[collected_list].prev_bkt_reg = next_obj_reg;
        }
        sph->collected_registers = next_obj_reg;
        // Next register in the list
        next_obj_reg = reg->next_obj_reg;
        G_ASSERT(next_obj_reg != handle,
                "Loop detected");
        reg->next_obj_reg = INVALID_INDEX;
    };
}
