/*
 * Copyright 2014, NICTA
 *
 * This software may be distributed and modified according to the terms of
 * the GNU General Public License version 2. Note that NO WARRANTY is provided.
 * See "LICENSE_GPLv2.txt" for details.
 *
 * @TAG(NICTA_GPL)
 */

#include <stdio.h>
#include <stdint.h>
#include <autoconf.h>

#include <sel4/sel4.h>
#include <sel4utils/util.h>
#include <vspace/vspace.h>
#include <sel4utils/mapping.h>

#include <sel4vchan/vmm_manager.h>
#include <sel4vchan/vchan_copy.h>
#include <sel4vchan/vchan_sharemem.h>
#include <sel4vchan/libvchan.h>
#include <sel4vchan/vchan_component.h>

#include <Vchan.h>

#define DEBUG_VCHAN

#ifdef DEBUG_VCHAN
#define DVCHAN(...) do{ printf("VCHAN: "); printf(__VA_ARGS__); }while(0)
#else
#define DVCHAN(...) do{}while(0)
#endif

#define USR_DCONNCT -2
#define USR_NOT_SET -1

/*
    State needed to represent a vchan instance in the vmm
*/
typedef struct vchan_connection {
    uint32_t domx, domy, port;
    uint32_t client_connected, server_connected;

    vchan_shared_mem_t *buffers;
    struct vchan_connection *next;
} vchan_instance_t;


static void init_buffer(void);
static void clear_buf(vchan_buf_t *b);

static int new_vchan_instance(vchan_connect_t *con);
static void rem_vchan_instance(vchan_instance_t *inst);
static int vchan_status(uint32_t domx, uint32_t domy, uint32_t port);
static int vm_side_closed(uint32_t domx, uint32_t domy, uint32_t port);

static vchan_shared_mem_t *alloc_buffer(void);

static vchan_buf_t *get_dom_buf(uint32_t buf, vchan_shared_mem_t *b);
static vchan_shared_mem_t *get_buffer(uint32_t id);
static vchan_instance_t *get_vchan_instance(uint32_t domx, uint32_t domy, uint32_t port);

static vchan_instance_t *first_inst = NULL;
static vchan_headers_t *headers = NULL; /* Shared dataport struct for intervm communication  */

void pre_init(void) {
    headers = (vchan_headers_t *) share_mem;
    headers->token = VCHAN_DATA_TOKEN;
    init_buffer();
}

static void clear_buf(vchan_buf_t *b) {
    assert(b != NULL);
    b->owner = -1;
    b->filled = 0;
    b->read_pos = 0;
    b->write_pos = 0;
}

/*
    Sets up initial values for data copying
*/
static void init_buffer(void) {
    int x, y;
    vchan_shared_mem_t *m;
    for(x = 0; x < NUM_BUFFERS; x++) {
        m = &headers->shared_buffers[x];
        m->alloced = 0;
        for(y = 0; y < 2; y++) {
            clear_buf(&m->bufs[y]);
        }
    }
}

int vchan_com_new_connection(vchan_connect_t con) {
    int res;
    res = new_vchan_instance(&con);
    return res;
}

int vchan_com_rem_connection(vchan_connect_t con) {
    vchan_instance_t *inst = get_vchan_instance(con.v.domain, con.v.dest, con.v.port);
    if(inst != NULL) {
        DVCHAN("vchan: closing down on %d side |%d\n", con.server, con.v.port);
        if(con.server)
            inst->server_connected = USR_DCONNCT;
        else
            inst->client_connected = USR_DCONNCT;

        if(inst->client_connected == USR_DCONNCT && inst->server_connected == USR_DCONNCT) {
            DVCHAN("vchan: SHUTTING DOWN CONNECTIOn\n");
            rem_vchan_instance(inst);
        } else {
            vchan_com_ping();
        }
    }
}


intptr_t vchan_com_get_buf(vchan_ctrl_t args, int cmd) {
    vchan_buf_t *b;

    vchan_instance_t *i = get_vchan_instance(args.domain, args.dest, args.port);
    if(i == NULL) {
        DVCHAN("vchan: failed to find instance\n");
        return 0;
    }

    if(cmd == VCHAN_RECV) {
        b = get_dom_buf(args.domain, i->buffers);
    } else {
        b = get_dom_buf(args.dest, i->buffers);
    }

    if(b == NULL) {
        DVCHAN("vchan: failed to find buffer for instance\n");
        return 0;
    }

    return (intptr_t) ( (void *) b - (void *) share_mem);
}

void vchan_com_ping() {
    vevent_cl_emit();
    vevent_sv_emit();
}

int vchan_com_status(vchan_ctrl_t args) {
    vchan_instance_t *inst = get_vchan_instance(args.domain, args.dest, args.port);
    if(inst == NULL) {
        return 0;
    }

    return vchan_status(args.domain, args.dest, args.port);
}

int vchan_com_data_stats(vchan_ctrl_t args, int *data_ready, int *buffer_space) {
    vchan_buf_t *b;

    vchan_instance_t *i = get_vchan_instance(args.domain, args.dest, args.port);
    if(i == NULL || data_ready == NULL || buffer_space == NULL) {
        DVCHAN("Cannot find vchan instance, both sides maybe closed\n");
        return 1;
    }

    b = get_dom_buf(args.dest, i->buffers);
    /* If the connection is closed in some way, we cannot write to the other side */
    if(vchan_status(args.domain, args.dest, args.port) != 1) {
        DVCHAN("stats: cannot write to other side\n");
        *buffer_space = 0;
    } else {
        assert(b != NULL);
        b->filled = abs(b->write_pos - b->read_pos);
        *buffer_space = VCHAN_BUF_SIZE - b->filled;
    }

    b = get_dom_buf(args.domain, i->buffers);
    assert(b != NULL);
    b->filled = abs(b->write_pos - b->read_pos);
    *data_ready = b->filled;

    /* If the vm side of the vchan connection has closed, we let the caller know  */
    if(vm_side_closed(args.domain, args.dest, args.port)) {
        return 1;
    }

    return 0;
}

static int vm_side_closed(uint32_t domx, uint32_t domy, uint32_t port) {
    vchan_instance_t *i = get_vchan_instance(domx, domy, port);
    if(i == NULL)
        return 1;

    /* Cannot find client in connection list, we must have disconnected */
    if(i->client_connected != domx && i->server_connected != domx)
        return 1;

    return 0;
}


/*
    Modifies an integer in a guest vm to notify a guest vm of changed state
*/
void vchan_alert_domain(vchan_instance_t *i) {
    assert(i != NULL);
    vchan_com_ping();
}

/*
    Creates a new vchan connection between two guests
*/
static int new_vchan_instance(vchan_connect_t *con) {
    vchan_shared_mem_t *buffers;
    uint32_t domx, domy, port, caller;

    domx = caller = con->v.domain;
    domy = con->v.dest;
    port = con->v.port;

    /* See if this connection already exists */
    vchan_instance_t *new = get_vchan_instance(domx, domy, port);
    if(new == NULL) {
        new = malloc(sizeof(vchan_instance_t));
        if(new == NULL) {
            return -1;
        }

        buffers = alloc_buffer();
        if(buffers == NULL) {
            DVCHAN("vchan: bad buffer allocation!\n");
            return -1;
        }

        new->domx = MIN(domx, domy);
        new->domy = MAX(domx, domy);

        new->server_connected = USR_NOT_SET;
        new->client_connected = USR_NOT_SET;

        new->buffers = buffers;

        for(int x = 0; x < 2; x++) {
            clear_buf(&new->buffers->bufs[x]);
        }

        new->port = port;
        new->next = first_inst;
        first_inst = new;
    }

    if(con->server) {
        DVCHAN("vchan: server connection %d|%d|%d established\n", domx, domy, port);
        new->server_connected = domx;
        new->buffers->bufs[0].owner = MIN(domx, domy);
        new->buffers->bufs[1].owner = MAX(domx, domy);
    } else {
        new->buffers->bufs[0].owner = MIN(domx, domy);
        new->buffers->bufs[1].owner = MAX(domx, domy);
        DVCHAN("vchan: client connection %d|%d|%d established\n", domx, domy, port);
        new->client_connected = domx;
    }

    vchan_alert_domain(new);
    return 0;
}

/*
    Returns the address of a buffer belonging to a vm guest domain
*/
static vchan_buf_t *get_dom_buf(uint32_t buf, vchan_shared_mem_t *b) {
    if(b->bufs[0].owner == buf) {
        return &b->bufs[0];
    } else if(b->bufs[1].owner == buf) {
        return &b->bufs[1];
    }

    return NULL;
}

/*
    Buffer manipulation functions
*/
static vchan_shared_mem_t *alloc_buffer(void) {
    int x;
    vchan_shared_mem_t *cur;
    for(x = 0; x < NUM_BUFFERS; x++) {
        cur = get_buffer(x);
        assert(cur != NULL);
        if(cur->alloced == 0) {
            cur->alloced = 1;
            return cur;
        }
    }
    return NULL;
}

/*
    Return the address of a given buffer
*/
static vchan_shared_mem_t *get_buffer(uint32_t id) {
    if(id >= NUM_BUFFERS) {
        DVCHAN("get_buffer: bad id\n");
        return NULL;
    }
    return &(headers->shared_buffers[id]);
}

/*
    Return the status of a given vchan instance
*/
static int vchan_status(uint32_t domx, uint32_t domy, uint32_t port) {
    int val = -1;
    vchan_instance_t *inst = get_vchan_instance(domx, domy, port);
    if(inst == NULL) {
        return 0;
    }

    /* Server active but client not yet connected */
    if (inst->client_connected == USR_NOT_SET && inst->server_connected > 0) {
        return 2;
    /* Full connection */
    } else if (inst->client_connected >= 0 && inst->server_connected >= 0) {
        return 1;
    }

    /* Closed connection */
    return 0;
}

/*
    Find and return a given vchan instance
*/
static vchan_instance_t *get_vchan_instance(uint32_t domx, uint32_t domy, uint32_t port) {
    vchan_instance_t *inst = first_inst;
    while(inst != NULL) {
        if(inst->domx == MIN(domx, domy)) {
            if(inst->domy == MAX(domx, domy) && port == inst->port) {
                return inst;
            }
        }
        inst = inst->next;
    }
    return NULL;
}

/*
    Free the memory of a given vchan instance
*/
static void rem_vchan_instance(vchan_instance_t *inst) {
    vchan_instance_t *prev = NULL;
    vchan_instance_t *find = first_inst;

    while(find != NULL) {
        if(inst == find) {
            if(prev != NULL) {
                prev->next = find->next;
            } else {
                first_inst = find->next;
            }
            DVCHAN("vchan: removing %d | %d | %d\n", inst->domx, inst->domy, inst->port);
            inst->buffers->alloced = 0;
            free(find);
            return;
        }
        prev = find;
        find = find->next;
    }
}
