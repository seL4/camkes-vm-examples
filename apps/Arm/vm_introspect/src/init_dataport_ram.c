/*
 * Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <camkes.h>
#include <sel4vm/guest_vm.h>
#include <sel4vm/guest_memory.h>
#include <sel4vm/guest_memory_helpers.h>
#include <sel4vm/guest_ram.h>
#include <sel4vmmplatsupport/guest_memory_util.h>
#include <vmlinux.h>

extern unsigned long linux_ram_base;
extern unsigned long linux_ram_size;

extern dataport_caps_handle_t memdev_handle;

static vm_frame_t dataport_memory_iterator(uintptr_t addr, void *cookie)
{
    cspacepath_t return_frame;
    vm_frame_t frame_result = { seL4_CapNull, seL4_NoRights, 0, 0 };

    uintptr_t frame_start = ROUND_DOWN(addr, BIT(seL4_PageBits));
    if (frame_start < linux_ram_base || frame_start > linux_ram_base + linux_ram_size) {
        ZF_LOGE("Error: Not dataport ram region");
        return frame_result;
    }

    int page_idx = (frame_start - linux_ram_base) / BIT(seL4_PageBits);
    frame_result.cptr = dataport_get_nth_frame_cap(&memdev_handle, page_idx);
    frame_result.rights = seL4_AllRights;
    frame_result.vaddr = frame_start;
    frame_result.size_bits = seL4_PageBits;
    return frame_result;
}

void init_ram_module(vm_t *vm, void *cookie)
{
    int err;

    err = vm_ram_register_at_custom_iterator(vm, linux_ram_base, linux_ram_size, dataport_memory_iterator, NULL);
    assert(!err);
}
