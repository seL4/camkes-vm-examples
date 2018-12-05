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
#ifndef VMLINUX_H
#define VMLINUX_H

#include <sel4utils/irq_server.h>

#if defined(CONFIG_PLAT_TK1)
#include "tk1_vmlinux.h"

#elif defined(CONFIG_PLAT_EXYNOS54XX)
#include "exynos5_vmlinux.h"

#else
#error "Unknown SoC"
#endif
#define MACH_TYPE_SPECIAL    ~0
#define MACH_TYPE            MACH_TYPE_SPECIAL

int plat_install_linux_devices(vm_t* vm);
int route_irqs(vm_t* vm, irq_server_t irq_server);
typedef struct {
    const char* linux_bin;
    const char* device_tree;
} pwr_token_t;
extern pwr_token_t pwr_token;

irq_handler_fn get_custom_irq_handler(irq_t irq) WEAK;

typedef struct vmm_module {
    const char *name;
    void *cookie;
    // Function called for setup.
    void (*init_module)(vm_t* vm, void* cookie);
} ALIGN(32) vmm_module_t;

/* Declare a module.
 * For now, we put the modules in a separate elf section. */
#define DEFINE_MODULE(_name, _cookie, _init_module) \
    __attribute__((used)) __attribute__((section("_vmm_module"))) vmm_module_t VMM_MODULE_ ##_name = { \
    .name = #_name, \
    .cookie = _cookie, \
    .init_module = _init_module, \
};

#endif /* VMLINUX_H */

