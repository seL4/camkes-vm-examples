/*
 * Copyright 2019, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(DATA61_BSD)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <vmlinux.h>
#include <sel4vm/guest_vm.h>
#include <sel4vmmplatsupport/drivers/cross_vm_connection.h>
#include <sel4vmmplatsupport/drivers/pci_helper.h>

extern vmm_pci_space_t *pci;

int cross_vm_connections_init(vm_t *vm, uintptr_t connection_base_addr, struct camkes_crossvm_connection *connections,
                              int num_connections)
{
    crossvm_handle_t *crossvm_connections = calloc(num_connections, sizeof(crossvm_handle_t));;
    if (!crossvm_connections) {
        return -1;
    }
    for (int i = 0; i < num_connections; i++) {
        crossvm_dataport_handle_t *dp_handle = calloc(1, sizeof(crossvm_dataport_handle_t));
        if (!dp_handle) {
            ZF_LOGE("Failed to initialse cross vm connection dataport %d", i);
            return -1;
        }

        dataport_caps_handle_t *handle = connections[i].handle;
        dp_handle->size = handle->get_size();
        dp_handle->num_frames = handle->get_num_frame_caps();
        dp_handle->frames = handle->get_frame_caps();

        crossvm_connections[i].dataport = dp_handle;
        crossvm_connections[i].emit_fn = connections[i].emit_fn;
        crossvm_connections[i].consume_id = connections[i].consume_badge;
        crossvm_connections[i].connection_name = connections[i].connection_name;
    }
    int ret = cross_vm_connections_init_common(vm, connection_base_addr, crossvm_connections, num_connections,
                                               pci, get_crossvm_irq_num);
    free(crossvm_connections);
    return ret;
}
