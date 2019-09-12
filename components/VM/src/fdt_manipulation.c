/*
 * Copyright 2019, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(DATA61_BSD)
 */

#include <libfdt.h>
#include <utils/util.h>

static int append_prop_with_cells(void *fdt, int offset,  uint64_t val, int num_cells, const char *name)
{
    int err;
    if (num_cells == 2) {
        err = fdt_appendprop_u64(fdt, offset, name, val);
    } else if (num_cells == 1) {
        err = fdt_appendprop_u32(fdt, offset, name, val);
    } else {
        ZF_LOGF("non-supported arch");
    }

    return err;
}

int fdt_generate_memory_node(void *fdt, unsigned long base, size_t size)
{
    int root_offset = fdt_path_offset(fdt, "/");
    int address_cells = fdt_address_cells(fdt, root_offset);
    int size_cells = fdt_address_cells(fdt, root_offset);

    int this = fdt_add_subnode(fdt, root_offset, "memory");
    int err = fdt_appendprop_string(fdt, this, "device_type", "memory");
    if (err) {
        return -1;
    }
    err = append_prop_with_cells(fdt, this, base, address_cells, "reg");
    if (err) {
        return -1;
    }
    err = append_prop_with_cells(fdt, this, size, size_cells, "reg");
    if (err) {
        return -1;
    }

    return 0;
}

int fdt_generate_chosen_node(void *fdt, const char *stdout_path, const char *bootargs)
{
    int root_offset = fdt_path_offset(fdt, "/");
    int this = fdt_add_subnode(fdt, root_offset, "chosen");
    int err = fdt_appendprop_string(fdt, this, "stdout-path", stdout_path);
    if (err) {
        return -1;
    }
    err = fdt_appendprop_string(fdt, this, "bootargs", bootargs);
    if (err) {
        return -1;
    }
    err = fdt_appendprop_string(fdt, this, "linux,stdout-path", stdout_path);
    if (err) {
        return -1;
    }

    return 0;
}

int fdt_append_chosen_node_with_initrd_info(void *fdt, unsigned long base, size_t size)
{
    int root_offset = fdt_path_offset(fdt, "/");
    int address_cells = fdt_address_cells(fdt, root_offset);
    int this = fdt_path_offset(fdt, "/chosen");
    int err = append_prop_with_cells(fdt, this, base, address_cells, "linux,initrd-start");
    if (err) {
        return -1;
    }
    err = append_prop_with_cells(fdt, this, base + size, address_cells, "linux,initrd-end");
    if (err) {
        return -1;
    }

    return 0;
}
