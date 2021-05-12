/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <camkes.h>
#include <camkes/dataport_caps.h>

#include <sel4vm/guest_vm.h>
#include <sel4vmmplatsupport/drivers/cross_vm_connection.h>
#include <sel4vmmplatsupport/drivers/pci_helper.h>
#include <crossvm.h>

/* these are defined in the dataport's glue code */
extern dataport_caps_handle_t dp1_handle;
extern dataport_caps_handle_t dp2_handle;

static struct camkes_crossvm_connection connections[] = {
    {&dp1_handle, ready_emit, {.id = -1, .reg_callback = NULL}},
    {&dp2_handle, NULL, { .id = 1, .reg_callback = done_reg_callback }}
};

int camkes_cross_vm_connections_init(vm_t *vm, vmm_pci_space_t *pci, seL4_CPtr irq_notification,
                                     uintptr_t connection_base_address)
{
    return cross_vm_connections_init(vm, connection_base_address, connections, ARRAY_SIZE(connections), pci,
                                     irq_notification);
}
