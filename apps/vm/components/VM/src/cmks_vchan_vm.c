/*
 * Copyright 2014, NICTA
 *
 * This software may be distributed and modified according to the terms of
 * the GNU General Public License version 2. Note that NO WARRANTY is provided.
 * See "LICENSE_GPLv2.txt" for details.
 *
 * @TAG(NICTA_GPL)
 */

#include "cmks_vchan_vm.h"

#include <vchan/vmm_manager.h>
#include <vchan/vchan_copy.h>
#include <vchan/vchan_sharemem.h>
#include <vchan/libvchan.h>
#include <vchan/vchan_component.h>

#include <sel4arm-vmm/vchan_vm_component.h>

#include <VM.h>

static int driver_connect(void *data, uint64_t cmd);

static int vchan_connect(void *data, uint64_t cmd);
static int vchan_close(void *data, uint64_t cmd);
static int vchan_buf_state(void *data, uint64_t cmd);
static int vchan_readwrite(void *data, uint64_t cmd);
static int vchan_state(void *data, uint64_t cmd);

static void vchan_callback(void *addr);
static void vchan_ack(void* token);

/* Function lookup table for handling requests */
static struct vmm_manager_ops {
    int (*op_func[NUM_VMM_OPS])(void *, uint64_t);
} vmm_manager_ops_table = {
    .op_func[VMM_CONNECT]           =   &driver_connect,
    .op_func[SEL4_VCHAN_CONNECT]    =   &vchan_connect,
    .op_func[SEL4_VCHAN_CLOSE]      =   &vchan_close,
    .op_func[VCHAN_SEND]            =   &vchan_readwrite,
    .op_func[VCHAN_RECV]            =   &vchan_readwrite,
    .op_func[SEL4_VCHAN_BUF]        =   &vchan_buf_state,
    .op_func[SEL4_VCHAN_STATE]      =   &vchan_state,
};

static vm_t *run_vmm;
static char driver_arg[1024];
static bool driver_connected = 0;
static virq_handle_t vchan_irq_handle;

/* vchan connection definition */
static camkes_vchan_con_t vchan_camkes_component = {
    .connect = &vchan_con_new_connection,
    .disconnect = &vchan_con_rem_connection,
    .get_buf = &vchan_con_get_buf,
    .status = &vchan_con_status,
    .alert_status = &vchan_con_alert_status,
    .reg_callback = &vevent_reg_callback,
    .alert = &vchan_con_ping,
    .dest_dom_number = 50,
    .source_dom_number = 0,
};

static void vchan_ack(void* token) {
}

static void vchan_callback(void *addr) {
    vchan_alert_t in_alert;
    vm_copyin(run_vmm, &in_alert, addr, sizeof(vchan_alert_t));

    camkes_vchan_con_t *con = get_vchan_con(run_vmm, in_alert.dest);
    if(con == NULL) {
        // DPRINTF(2, "Domain %d, has no vchan component instance\n", in_alert.dest);
        return;
    }

    vchan_ctrl_t ct = {
        .dest = in_alert.dest,
        .port = in_alert.port,
    };

    in_alert.alert = con->alert_status(ct);

    vm_inject_IRQ(vchan_irq_handle);
    vm_copyout(run_vmm, &in_alert, addr, sizeof(vchan_alert_t));
    con->reg_callback(&vchan_callback, addr);
}

void vm_vchan_setup(vm_t *vm) {
    vchan_irq_handle = vm_virq_new(vm, VCHAN_EVENT_IRQ, &vchan_ack, NULL);
    reg_new_vchan_con(vm, &vchan_camkes_component);
    vchan_camkes_component.data_buf = (void *)share_mem;
}

/*
    Return the given vm guest number of this component
*/
int get_vm_num() {
    int res;
    // char *name = (char *) get_instance_name();
    int ret = sscanf("vm_vm0", "vm_vm%d", &res);
    if(ret == 0) {
        // DPRINTF(2, "vchan_driver: failed to get run num\n");
        return -1;
    }

    return res;
}


/*
    Return the state of a given vchan connection
*/
static int vchan_state(void *data, uint64_t cmd) {
    vchan_check_args_t *args = (vchan_check_args_t *)data;
    camkes_vchan_con_t *con = get_vchan_con(run_vmm, args->v.dest);
    if(con == NULL) {
        // DPRINTF(2, "Domain %d, has no vchan component instance\n", args->v.domain);
        return -1;
    }

    args->state = con->status(args->v);
    return 0;
}

/*
    Connect a vchan to a another guest vm
*/
static int vchan_connect(void *data, uint64_t cmd) {
    int res;
    vmm_args_t *args = (vmm_args_t *)data;
    vchan_connect_t *pass = (vchan_connect_t *)args->ret_data;

    camkes_vchan_con_t *con = get_vchan_con(run_vmm, pass->v.dest);
    if(con == NULL) {
        // DPRINTF(2, "Domain %d, has no vchan component instance\n", pass->v.domain);
        return -1;
    }

    vchan_connect_t t = {
        .v.domain = con->source_dom_number,
        .v.dest = pass->v.dest,
        .v.port = pass->v.port,
        .server = pass->server,
    };

    res = con->connect(t);
    if(res < 0) {
        // DPRINTF(2, "Domain %d, failed to connect to \n", pass->v.domain, pass->v.dest);
        return -1;
    }
    res = con->reg_callback(&vchan_callback, (void *) pass->event_mon);
    if(res < 0) {
        // DPRINTF(2, "Domain %d, failed reg callback\n", pass->v.domain);
        return -1;
    }

    return 0;
}

/*
    Close a vchan connection this guest vm is using
*/
static int vchan_close(void *data, uint64_t cmd) {
    // panic("init-side vchan close not implemented!");
    // vmm_args_t *args = (vmm_args_t *)data;
    // vchan_connect_t *pass = (vchan_connect_t *)args->ret_data;
    // uint32_t domx = get_vm_num();
    // uint32_t domy = pass->v.domain;
    // ctrl_rem_vchan_connection(domx, domy, pass->server, pass->v.port);
    return 0;
}


/*
    Function for sending/recieving data from other vm's
        Copies into a memory buffer, and then defers to VmmManager to finish the operation
        Defering is necessary for ensuring concurrency
*/
static int vchan_readwrite(void *data, uint64_t cmd) {
    vchan_args_t *args = (vchan_args_t *)data;
    // vspace_t *vs = &run_vmm->vmm->guest_mem.vspace;
    int *update;

    camkes_vchan_con_t *con = get_vchan_con(run_vmm, args->v.dest);
    if(con == NULL) {
        // DPRINTF(2, "Domain %d, has no vchan component instance\n", args->v.domain);
        return -1;
    }

    size_t size = args->size;
    uintptr_t phys = args->mmap_phys_ptr;

    vchan_ctrl_t bargs = {
        .domain = con->source_dom_number,
        .dest = args->v.dest,
        .port = args->v.port,
    };

    // /* Perfom copy of data to appropriate destination */
    vchan_buf_t *b = get_vchan_buf(&bargs, con, cmd);
    assert(b != NULL);
    size_t filled = abs(b->read_pos - b->write_pos);

    /*
        If streaming, send as much data as possible
         If not streaming, any operation that can't fit into the buffer fails
    */
    if(cmd == VCHAN_RECV) {
        if(args->stream) {
            args->size = MIN(filled, args->size);
        } else if(args->size > filled) {
            return -1;
        }
    } else {
        if(args->stream) {
            args->size = MIN(VCHAN_BUF_SIZE - filled, args->size);
        } else if (args->size > (VCHAN_BUF_SIZE - filled)) {
            return -1;
        }
    }

    if(cmd == VCHAN_SEND) {
        update = &(b->write_pos);
    } else {
        update = &(b->read_pos);
    }

    off_t start = (*update % VCHAN_BUF_SIZE);
    off_t remain = 0;
    if(start + size > VCHAN_BUF_SIZE) {
        remain = (start + size) - VCHAN_BUF_SIZE;
        size -= remain;
    }

    if(cmd == VCHAN_SEND) {
        vm_copyin(run_vmm, (b->sync_data + start), phys, size);
        vm_copyin(run_vmm, (b->sync_data), phys, remain);
    } else {
        vm_copyout(run_vmm, (b->sync_data + start), phys, size);
        vm_copyout(run_vmm, (b->sync_data + start), phys, size);
    }

    *update += (size + remain);
    con->alert();
    // DPRINTF(4, "vmcall_readwrite: finished action %d | %d | %d\n", (int) cmd, size, (int) remain);

    args->size = (size + remain);
    return 0;
}


/*
    See the state of a given vchan buffer
        i.e. how much data is in the buffer, how much can be written into the buffer
*/
static int vchan_buf_state(void *data, uint64_t cmd) {
    vchan_check_args_t *args = (vchan_check_args_t *)data;

    camkes_vchan_con_t *con = get_vchan_con(run_vmm, args->v.dest);
    if(con == NULL) {
        // DPRINTF(2, "Domain %d, has no vchan component instance\n", args->v.domain);
        return -1;
    }

    vchan_ctrl_t bargs = {
        .domain = con->source_dom_number,
        .dest = args->v.dest,
        .port = args->v.port,
    };

    /* Perfom copy of data to appropriate destination */
    vchan_buf_t *b = get_vchan_buf(&bargs, con, cmd);
    assert(b != NULL);
    size_t filled = abs(b->read_pos - b->write_pos);
    if(args->checktype == NOWAIT_DATA_READY) {
        args->state = filled;
    } else {
        args->state = VCHAN_BUF_SIZE - filled;
    }

    return 0;
}


/*
    Used for replying back to a driver successfully connecting
*/
static int driver_connect(void *data, uint64_t cmd) {
    /* Only allow one vchan driver instance to be connected */
    if(driver_connected)
        return -1;
    struct vmm_args *vargs = (struct vmm_args *)data;

    driver_connected = 1;
    vargs->datatype = DATATYPE_INT;
    int *res = (int *)vargs->ret_data;
    *res = get_vm_num();
    if(*res < 0) {
        return -1;
    }

    // printf("vchan_driver_connect: num is %d\n", *res);
    return 0;
}

void vchan_entry_point(vm_t *vm, uint32_t data) {
	char args_buffer[256];
	run_vmm = vm;
    int cmd;

	vm_copyin(vm, &args_buffer, data, sizeof(vmcall_args_t));
	vmcall_args_t *args = (vmcall_args_t *)args_buffer;
    cmd = args->cmd;

    // printf("vchan_call: %d\n", args->cmd);
     /* Catch if the request is for an invalid command */
    if(cmd >= NUM_VMM_OPS || vmm_manager_ops_table.op_func[cmd] == NULL) {
        // DPRINTF(2, "vchan: unsupported command or null tbl arg %d\n", cmd);
        args->err = -1;
    } else {
        /* Perform given token:command action */
		vm_copyin(vm, &driver_arg, args->phys_data, args->size);
        args->err = (*vmm_manager_ops_table.op_func[cmd])(&driver_arg, cmd);
        if(args->err != -1) {
			vm_copyout(vm, &driver_arg, args->phys_data, args->size);
        }
    }
    // printf("vchan_call_conclude: %d - %d\n", cmd, args->err);
	vm_copyout(vm, &args_buffer, data, sizeof(vmcall_args_t));
}
