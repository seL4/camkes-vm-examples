/*
 * Copyright 2017, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the GNU General Public License version 2. Note that NO WARRANTY is provided.
 * See "LICENSE_GPLv2.txt" for details.
 *
 * @TAG(DATA61_GPL)
 */

#include <camkes.h>
#include <dataport_caps.h>
#include <cross_vm_dataport.h>
#include <sel4vm/guest_vm.h>
#include <sel4vmmplatsupport/drivers/pci_helper.h>
#include <pci/helper.h>

// these are defined in the dataport's glue code
extern dataport_caps_handle_t dp1_handle;
extern dataport_caps_handle_t dp2_handle;

static dataport_caps_handle_t *dataports[] = {
    &dp1_handle,
    &dp2_handle,
};

int cross_vm_dataports_init(vm_t *vm, vmm_pci_space_t *pci)
{
    return cross_vm_dataports_init_common(vm, dataports, sizeof(dataports) / sizeof(dataports[0]),
                                          pci);
}
