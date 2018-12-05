/*
 * Copyright 2018, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(DATA61_BSD)
 */

#include <camkes.h>
#include <sel4arm-vmm/vm.h>
#include <vmlinux.h>
extern int start_extra_frame_caps;

static void map_frame_hack_init_module(vm_t *vm, void *cookie) {

    /* hack to give access to other components
       see https://github.com/smaccm/vm_hack/blob/master/details.md for details */
    int offset = 0;
    for (int i = 0; i < num_extra_frame_caps; i++) {
        int err = vm_map_frame(vm, start_extra_frame_caps + i,
            extra_frame_map_address + offset, PAGE_BITS_4K, 1, seL4_AllRights);
        assert(!err);
        offset += PAGE_SIZE;
    }
}

DEFINE_MODULE(map_frame_hack, NULL, map_frame_hack_init_module)
