/*
 * Copyright 2014, NICTA
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(NICTA_BSD)
 */

#include <autoconf.h>

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <allocman/allocman.h>
#include <allocman/bootstrap.h>
#include <allocman/vka.h>
#include <vka/capops.h>
#include <vka/object.h>

#include <vspace/vspace.h>
#include <simple/simple.h>
#include <simple/simple_helpers.h>
#include <simple-default/simple-default.h>
#include <platsupport/io.h>
#include <sel4platsupport/platsupport.h>
#include <sel4platsupport/io.h>

#include <cpio/cpio.h>

#include <sel4arm-vmm/vm.h>

#include "vmlinux.h"

#define VM_PRIO             100
#define VM_BADGE            (1U << 0)
#define VM_LINUX_NAME       "linux"
#define VM_LINUX_DTB_NAME   "linux-dtb"
#define VM_NAME             "Linux"

#ifndef DEBUG_BUILD
#define seL4_DebugHalt() do{ printf("Halting...\n"); while(1); } while(0)
#endif

vka_t _vka;
simple_t _simple;
vspace_t _vspace;
sel4utils_alloc_data_t _alloc_data;
allocman_t *allocman;
static char allocator_mempool[8388608];
seL4_CPtr _fault_endpoint;


struct ps_io_ops _io_ops;

extern char _cpio_archive[];


static void
print_cpio_info(void)
{
    struct cpio_info info;
    const char* name;
    unsigned long size;
    int i;

    cpio_info(_cpio_archive, &info);

    printf("CPIO: %d files found.\n", info.file_count);
    assert(info.file_count > 0);
    for (i = 0; i < info.file_count; i++) {
        void * addr;
        char buf[info.max_path_sz + 1];
        buf[info.max_path_sz] = '\0';
        addr = cpio_get_entry(_cpio_archive, i, &name, &size);
        assert(addr);
        strncpy(buf, name, info.max_path_sz);
        printf("%d) %-20s  0x%08x, %8ld bytes\n", i, buf, (uint32_t)addr, size);
    }
    printf("\n");
}

void camkes_make_simple(simple_t *simple);


static int
vmm_init(void)
{
    vka_object_t fault_ep_obj;
    vka_t* vka;
    simple_t* simple;
    vspace_t* vspace;
    int err;

    vka = &_vka;
    vspace = &_vspace;
    simple = &_simple;
    fault_ep_obj.cptr = 0;

    /* Camkes adds nothing to our address space, so this array is empty */
    void *existing_frames[] = {
        NULL
    };

    camkes_make_simple(simple);

    /* Initialize allocator */
    allocman = bootstrap_use_current_1level(
            simple_get_cnode(simple),
            simple_get_cnode_size_bits(simple),
            simple_last_valid_cap(simple) + 1,
            BIT(simple_get_cnode_size_bits(simple)),
            sizeof(allocator_mempool), allocator_mempool
    );
    assert(allocman);
    err = allocman_add_simple_untypeds(allocman, simple);
    assert(!err);
    allocman_make_vka(vka, allocman);

    /* Initialize the vspace */
    err = sel4utils_bootstrap_vspace(vspace, &_alloc_data,
            simple_get_init_cap(simple, seL4_CapInitThreadPD), vka, NULL, NULL, existing_frames);
    assert(!err);

    /* Initialise device support */
    err = sel4platsupport_new_io_mapper(*simple, *vspace, *vka,
                                        &_io_ops.io_mapper);
    assert(!err);

    /* Allocate a endpoint for listening to events */
    err = vka_alloc_endpoint(vka, &fault_ep_obj);
    assert(!err);
    _fault_endpoint = fault_ep_obj.cptr;

    return 0;
}

#define RAM_START 0x40000000
#define RAM_END   0x60000000

static void
map_unity_ram(vm_t* vm)
{
    int err;

    uintptr_t start;
    reservation_t res;
    unsigned int bits = 21;
    res = vspace_reserve_range_at(&vm->vm_vspace, (void*)RAM_START, RAM_END - RAM_START, seL4_AllRights, 1);
    assert(res.res);
    for (start = RAM_START;; start += BIT(bits)) {
        cspacepath_t frame;
        err = vka_cspace_alloc_path(vm->vka, &frame);
        assert(!err);
        err = simple_get_frame_cap(vm->simple, (void*)start, bits, &frame);
        if (err) {
            vka_cspace_free(vm->vka, frame.capPtr);
            break;
        }
        printf("Adding passthrough frame at 0x%x\n", start);
        err = vspace_map_pages_at_vaddr(&vm->vm_vspace, &frame.capPtr, &bits, (void*)start, 1, bits, res);
        assert(!err);
    }
}

int
main_continued(void)
{
    struct vm vm;
    int err;

    err = vmm_init();
    assert(!err);

    print_cpio_info();

    /* Create the VM */
    err = vm_create(VM_NAME, VM_PRIO, _fault_endpoint, VM_BADGE,
                    &_vka, &_simple, &_vspace, &_io_ops, &vm);
    if (err) {
        printf("Failed to create VM\n");
        seL4_DebugHalt();
        return -1;
    }

    /* HACK: See if we have a "RAM device" for 1-1 mappings */
    map_unity_ram(&vm);

    /* Load system images */
    printf("Loading Linux: \'%s\' dtb: \'%s\'\n", VM_LINUX_NAME, VM_LINUX_DTB_NAME);
    err = load_linux(&vm, VM_LINUX_NAME, VM_LINUX_DTB_NAME);
    if (err) {
        printf("Failed to load VM image\n");
        seL4_DebugHalt();
        return -1;
    }

    /* Power on */
    printf("Starting VM\n\n");
    err = vm_start(&vm);
    if (err) {
        printf("Failed to start VM\n");
        seL4_DebugHalt();
        return -1;
    }

    /* Loop forever, handling events */
    while (1) {
        seL4_MessageInfo_t tag;
        seL4_Word sender_badge;

        tag = seL4_Wait(_fault_endpoint, &sender_badge);
        assert(sender_badge == VM_BADGE);

        err = vm_event(&vm, tag);
        if (err) {
            /* Shutdown */
            vm_stop(&vm);
            seL4_DebugHalt();
        }
    }

    return 0;
}

void run(void) {
    printf("Calling main, expect failure\n");
    main_continued();
}

