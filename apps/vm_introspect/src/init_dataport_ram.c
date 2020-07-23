/*
 * Copyright 2020, Data61
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
#include <sel4vm/guest_vm.h>
#include <sel4vm/guest_memory.h>
#include <sel4vm/guest_memory_helpers.h>
#include <sel4vm/guest_ram.h>
#include <sel4vmmplatsupport/guest_memory_util.h>
#include <vmlinux.h>

extern unsigned long linux_ram_base;
extern unsigned long linux_ram_size;

extern dataport_caps_handle_t memdev_handle;

struct dataport_iterator_cookie {
    cspacepath_t *dataport_frames;
    vm_t *vm;
};

static vm_frame_t dataport_memory_iterator(uintptr_t addr, void *cookie){
    cspacepath_t return_frame;
    vm_frame_t frame_result = { seL4_CapNull, seL4_NoRights, 0, 0 };
    struct dataport_iterator_cookie *dataport_cookie = (struct dataport_iterator_cookie *)cookie;
    cspacepath_t *dataport_frames = dataport_cookie->dataport_frames;
    vm_t *vm = dataport_cookie->vm;

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

static int get_dataport_frames(vm_t * vm, cspacepath_t **dataport_frames){
    unsigned int num_pages = dataport_get_num_frame_caps(&memdev_handle);

    cspacepath_t *ram_frames_paths = (cspacepath_t *)calloc(1, num_pages * sizeof(cspacepath_t));
    assert(ram_frames_paths != NULL);
    for (uint32_t i =0; i < num_pages; i++){
        seL4_CPtr cap =  dataport_get_nth_frame_cap(&memdev_handle, i);
        vka_cspace_make_path(vm->vka, cap, &ram_frames_paths[i]);
    }
    memset(memdev, dataport_get_size(&memdev_handle), 0);
    *dataport_frames = ram_frames_paths;
    return 0;
}

static int vm_ram_register_dataport(vm_t *vm, uintptr_t start, size_t bytes){
    vm_memory_reservation_t *ram_reservation;
    int err;

    cspacepath_t * dataport_frames;
    err = get_dataport_frames(vm, &dataport_frames);
    if (err){
        ZF_LOGE("Failed to get dataport frames");
        return -1;
    }

    struct dataport_iterator_cookie *dataport_cookie = calloc(1, sizeof(struct dataport_iterator_cookie));
    if (!dataport_cookie) {
        ZF_LOGE("Failed to allocate dataport cookie");
        return -1;
    }
    dataport_cookie->vm = vm;
    dataport_cookie->dataport_frames = dataport_frames;
    return vm_ram_register_at_custom_iterator(vm, start, bytes, dataport_memory_iterator, (void *)dataport_cookie);
}

void init_ram_module(vm_t *vm, void *cookie)
{
    int err;

    err = vm_ram_register_dataport(vm, linux_ram_base, linux_ram_size);
    assert(!err);
}
