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

void grid_register_list_init_register(
        size_t index,
        size_t bucket,
        uint32_t level,
        size_t reg_head)
{

}


