#ifndef GEOMETRY_GRID_REGISTER_H
#define  GEOMETRY_GRID_REGISTER_H
#include <stdlib.h>
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
    size_t bucket; /** The register is in this bucket. */
    int32_t level; /** The level this register belongs to. */
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
 * Allocates a new register or reuse a collected one.
 * param grs List the register is created from.
 * param _reg Index of the allocated register.
 * return ec_no_error on success ec_memory error otherwise.
 */
enum error_code_e grid_register_list_new_register(
        _IN struct grid_register_list_st * grs,
        _OUT size_t * _reg);

#endif
