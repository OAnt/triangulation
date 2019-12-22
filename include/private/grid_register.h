#ifndef GEOMETRY_GRID_REGISTER_H
#define  GEOMETRY_GRID_REGISTER_H
#include <stdint.h>
#include <stdlib.h>
#include <private/spatial_index_declarations.h>
#include <public/common.h>

/**
 * Member of the linked list of objects stored in
 * a bucket.
 */
struct grid_register_st{
    size_t index; /** Index of the stored object. */
    size_t next_obj_reg; /** Index of the next register
                           for the inserted object if it
                           spans over multiple bucket. */
    size_t next_bkt_reg; /** Index of the next register
                           in the bucket. */
    size_t prev_bkt_reg; /** Index of the previous register
                           in the bucket. */
    struct spatial_index_key_st key; /** Holds information about
                                       were this register is */
    //size_t bucket; [>* The register is in this bucket. <]
    //uint32_t level; [>* The level this register belongs to. <]
};

/**
 * Collects the underlying array's unused elements to
 * reuse them when needed.
 */
struct grid_register_list_st{
    struct grid_register_st * registers; /** Array of registers (not iterable
                                           some may have been collected) */
    size_t collected_registers; /** Index of the first register in the
                                  linked list of unused registers */
};

/**
 * Initializes a grid register list, initialize before any usage.
 * param grs Grid register list to initialize.
 * return ec_no_error on success, ec_memory_error otherwise
 */
enum error_code_e grid_register_list_init(
        _IN struct grid_register_list_st * grs);

/**
 * Deallocates any memory used in the provided grid register list.
 * param grs List to clean.
 * return nothing.
 */
void grid_register_list_cleanup(
        _IN struct grid_register_list_st * grs);

/**
 * Removes a register from its linked list, this function assumes reg is in grs.
 * param grs List reg belongs to.
 * param reg The register to remove.
 * return nothing.
 */
void grid_register_remove_from_list(
        _IN struct grid_register_list_st * grs,
        _IN struct grid_register_st * reg);

/**
 * Adds the register at reg_index at the beginning of the linked list of
 * registers that starts at head_index.
 * param grs List reg and head belongs to.
 * param reg_index Index of the register to add.
 * param reg_index Index of the first register in the list.
 * param obj_list Head of the linked list of registers that an inserted object
 * return nothing
 */
void grid_register_prepend_to_lists(
        struct grid_register_list_st * grs,
        size_t reg_index,
        size_t head_index,
        size_t * obj_list);

/**
 * Allocates a new register or reuse a collected one.
 * param grs List the register is created from.
 * param _reg Index of the allocated register.
 * return ec_no_error on success ec_memory error otherwise.
 */
enum error_code_e grid_register_list_new_register(
        _IN struct grid_register_list_st * grs,
        _OUT size_t * _reg);

/**
 * Prototype for a caller issued function. Called upon
 * removing a register.
 * param reg Register that is being removed
 * param data Caller issued pointer
 * return nothing.
 */
typedef void (*register_remove_by_handle_callback_f)(
        struct grid_register_st * reg,
        void * data);

/**
 * Removes all the registers that corresponds to a given
 * handle.
 * param grs List the registers belong to.
 * param handle Handle of the registers to remove.
 * param callback Function called upon removing a register.
 * param data Caller issued pointer forwarded to the callback.
 * return nothing.
 */
void grid_register_list_remove_registers_by_handle(
        struct grid_register_list_st * grs,
        size_t handle,
        register_remove_by_handle_callback_f callback,
        void * data);

#endif
