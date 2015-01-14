/*
 * Copyright 2014, NICTA
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(NICTA_BSD)
 */

#ifndef VMLINUX_H
#define VMLINUX_H

#include <sel4arm-vmm/vm.h>

int load_linux(vm_t* vm, const char* kernel_name, const char* dtb_name);

#endif /* VMLINUX_H */

