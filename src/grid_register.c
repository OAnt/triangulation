#include <private/array.h>
#include <private/debug.h>
#include <private/grid_register.h>

enum error_code_e grid_register_list_init(
        struct grid_register_list_st * grs)
{
    enum error_code_e err = array_new(
            struct grid_register_st, 0, &grs->registers);
    if(err != ec_no_error) return err;
    grs->collected_registers = INVALID_INDEX;
    return ec_no_error;
}

void grid_register_list_cleanup(
        struct grid_register_list_st * grs)
{
    array_delete(&grs->registers);
    memset(grs, 0, sizeof(struct grid_register_list_st));
}

void grid_register_remove_from_list(
        struct grid_register_list_st * grs,
        struct grid_register_st * reg)
{
    G_ASSERT((reg - grs->registers) < array_length(grs->registers),
            "reg is not a part of grs");
    size_t previous_index = reg->prev_bkt_reg;
    size_t next_index = reg->next_bkt_reg;
    if(next_index != INVALID_INDEX){
        grs->registers[next_index].prev_bkt_reg = previous_index;
    }
    if(previous_index != INVALID_INDEX){
        grs->registers[previous_index].next_bkt_reg = next_index;
    }
}

//spatial_hash_new_register
enum error_code_e grid_register_list_new_register(
        struct grid_register_list_st * grs,
        size_t * _reg)
{
    struct grid_register_st * reg;
    // a register is available (already removed)
    if(grs->collected_registers != INVALID_INDEX){
        reg = &grs->registers[grs->collected_registers];
        G_ASSERT(reg->prev_bkt_reg == INVALID_INDEX &&
                reg->next_obj_reg == INVALID_INDEX &&
                reg->index == INVALID_INDEX &&
                reg->bucket == INVALID_INDEX,
                "A collected element was not uninitialized");
        *_reg = grs->collected_registers;;
        // removing the object from the list is in (list of unused)
        grs->collected_registers = reg->next_bkt_reg;
        grid_register_remove_from_list(grs, reg);
    // No register available in the list of previously removed
    // allocating
    }else{
        size_t index = array_length(grs->registers);
        enum error_code_e err = array_resize(&grs->registers, index + 1);
        if(err != ec_no_error) return err;
        *_reg = index;
    }
    return ec_no_error;
}

void grid_register_prepend_to_lists(
        struct grid_register_list_st * grs,
        size_t reg_index,
        size_t head_index,
        size_t * obj_list)
{
    G_ASSERT(reg_index < array_length(grs->registers) && 
            (head_index < array_length(grs->registers) ||
             head_index == INVALID_INDEX),
            "Registers are out of bounds");
    struct grid_register_st * reg = &grs->registers[reg_index];
    // Adding (prepending) the register to the bucket that 
    // was found to intersect it, this is a
    // two way list to allow easy removal for the
    // bucket list.
    reg->next_bkt_reg = head_index;
    reg->prev_bkt_reg = INVALID_INDEX;
    if(reg->next_bkt_reg != INVALID_INDEX){
        G_ASSERT(grs->registers[head_index].prev_bkt_reg == INVALID_INDEX,
                "Not the head");
        grs->registers[reg->next_bkt_reg].prev_bkt_reg = reg_index;
    }
    // Adding the object to the one way linked list
    // of objects that corresponds to the index
    reg->next_obj_reg = *obj_list;
    // An handle (head of the list)
    // is returned to the caller in order to allow
    // removal of the object from the spatial hash
    // as the object (index) may be spread over
    // several buckets
    *obj_list = reg_index;
}

