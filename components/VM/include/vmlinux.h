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

#include <sel4utils/irq_server.h>

#include <plat/vmlinux.h>

#define MACH_TYPE_SPECIAL    ~0
#define MACH_TYPE            MACH_TYPE_SPECIAL

irq_callback_fn_t get_custom_irq_handler(ps_irq_t irq) WEAK;

/* Struct type that's passed into the IRQ callback functions for
 * this component */
struct irq_token {
    vm_t *vm;
    ps_irq_t irq;
    int virq;
    ps_irq_acknowledge_fn_t acknowledge_fn;
    void *ack_data;
};
typedef struct irq_token *irq_token_t;

typedef struct vmm_module {
    const char *name;
    void *cookie;
    // Function called for setup.
    void (*init_module)(vm_t *vm, void *cookie);
} ALIGN(32) vmm_module_t;

/* Declare a module.
 * For now, we put the modules in a separate elf section. */
#define DEFINE_MODULE(_name, _cookie, _init_module) \
    __attribute__((used)) __attribute__((section("_vmm_module"))) vmm_module_t VMM_MODULE_ ##_name = { \
    .name = #_name, \
    .cookie = _cookie, \
    .init_module = _init_module, \
};
