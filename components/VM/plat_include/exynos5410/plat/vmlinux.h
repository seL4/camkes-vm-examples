/*
 * Copyright 2017, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(DATA61_BSD)
 */
#pragma once

#include <sel4arm-vmm/vm.h>

#define VUSB_ADDRESS         0x33330000
#define VUSB_IRQ             198
#define VUSB_NINDEX          5
#define VUSB_NBADGE          0x123


#define LINUX_RAM_BASE    0x40000000
#define LINUX_RAM_PADDR_BASE LINUX_RAM_BASE
#define LINUX_RAM_SIZE    0x40000000
#define PLAT_RAM_END      0xc0000000
#define LINUX_RAM_OFFSET  0
#define DTB_ADDR          (LINUX_RAM_BASE + 0x0F000000)
#define INITRD_MAX_SIZE   0x1900000 //25 MB
#define INITRD_ADDR       (DTB_ADDR - INITRD_MAX_SIZE) //0x4D700000

static const int linux_pt_irqs[] = {
    27, 85, 107, 109,
#ifndef FEATURE_VUSB
    103
#endif
};

void vusb_notify(void);
