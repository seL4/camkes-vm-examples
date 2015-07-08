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

#include <sel4vchan/vmm_manager.h>
#include <sel4vchan/vchan_copy.h>
#include <sel4vchan/vchan_sharemem.h>
#include <sel4vchan/libvchan.h>
#include <sel4vchan/vchan_component.h>

#include "vchan_testsuite.h"

#include <helloworld.h>
#include <camkes/dataport.h>

static camkes_vchan_con_t con = {
    .connect = &vchan_con_new_connection,
    .disconnect = &vchan_con_rem_connection,
    .get_buf = &vchan_con_get_buf,
        .status = &vchan_con_status,

    .alert = &vchan_con_ping,
    .wait = &vevent_wait,
    .poll = &vevent_poll,

    .dest_dom_number = 0,
    .source_dom_number = 50,
};

#ifdef VCHAN_COMPONENT_DEBUG_OUTPUT
#define DHELL(...) do{ printf("vchantests_comp: "); printf(__VA_ARGS__); }while(0)
#else
#define DHELL(...) do{}while(0)
#endif

/*
    TESTSUITE:
        Tests, CAMmkES component side
*/

/*
    vchantests_handshake:
        Simple writing of int and then waiting for an ack
        If this doesn't work, then nothing else will work
*/
int vchantests_handshake(libvchan_t *ctrl) {
    int handshake = -1;
    libvchan_wait(ctrl);

    libvchan_read(ctrl, &handshake, sizeof(int));
    if(handshake != VCHANTESTS_HANDSHAKE_CODE)
        return -1;
    handshake = VCHANTESTS_HANDSHAKE_ACK;
    libvchan_write(ctrl, &handshake, sizeof(int));

    return 0;
}

/*
    vchantests_prod_cons:
        Simple buffer reading and writing test, similar to packet test
*/
int vchantests_prod_cons(libvchan_t *ctrl) {
    int sz, x, i, failed, buf[PROD_CONS_CHUNK];
    for(x = 0; x < PROD_CONS_NUM; x++) {
        failed = 0;
        sz = libvchan_recv(ctrl, buf, PROD_CONS_CHUNK * sizeof(int));
        if(sz == PROD_CONS_CHUNK * sizeof(int)) {
            for(i = 0; i < PROD_CONS_CHUNK; i++) {
                if(i + x != buf[i]) {
                    DHELL("with: %d !packet_mismatch! got:%d:expected:%d\n", x, buf[i], i);
                    failed = 1;
                }
            }
        } else {
            DHELL("with: %d !incorrect size! got:%d:expected:%d\n", x, sz, PROD_CONS_CHUNK);
            failed = 1;
        }
        assert(failed == 0);
    }

    x = 1;
    libvchan_write(ctrl, &x, sizeof(x));

    return 0;
}

/*
    vchantests_funnel:
        Recieve a an entire buffer, that is sent by the client in chunks
        We should block until the client is finished writing everything
        Process is then repeated in reverse
*/
int vchantests_funnel(libvchan_t *ctrl) {
    int sz, count;
    libvchan_recv(ctrl, &sz, sizeof(int));
    char *buffer, c;

    buffer = malloc(sz);
    assert(buffer);

    libvchan_wait(ctrl);
    assert(libvchan_data_ready(ctrl) > 0);
    libvchan_recv(ctrl, buffer, sz);
    assert(libvchan_data_ready(ctrl) == 0);

    c = 0;
    for(count = 0; count != sz; count++) {
        assert(buffer[count] == c);
        c++;
    }

    DHELL("funnel: ok buffer :), writing back\n");
    for(count = 0; count < sz; count++) {
        libvchan_send(ctrl, &(buffer[count]), sizeof(char));
    }

    return 0;
}

/*
    vchantests_vm_burst - initialise_buffer:
        Initialise data that will be sent back to client
*/
static void initialise_buffer(int *buf, int sz) {
    int x, i = 0;
    int key = 0;
    for(x = 0; x < sz; x++) {
        if(x % VM_BURST_CHUNK_SIZE == 0) {
            key = vm_burst_base_nums[i % VM_BURST_NUM_SLEEPS];
            i++;
        }
        buf[x] = x + key;
    }
    buf[sz - 1] = VM_BURST_CHECKSUM;
}


/*
    vchantests_vm_burst:
        Send data a client that sleeps intermittently

    TODO
        - Make this more stressful, should send more data
        - Make sure the client checks data recieved, should be correct
*/
int vchantests_vm_burst(libvchan_t *ctrl) {
    int sz = VM_BURST_TOTAL_SZ;
    int buffer[sz];
    int pos = 0;
    initialise_buffer(buffer, sz);

    DHELL("vm_burst: planning to write %d bytes to vm client\n", sz);
    while(sz > 0) {
        int write = MIN(VM_BURST_CHUNK_SIZE, sz);
        sz -= write;
        libvchan_write(ctrl, buffer + pos, write);
        DHELL("vm_burst: wrote %d|%d bytes to vm client\n", write, sz);
        pos += write;
    }

    DHELL("vm_burst: waiting for ack\n");
    libvchan_recv(ctrl, &pos, sizeof(int));
    DHELL("vm_burst: done\n");

    return 0;
}

/*
    vchantests_bigwrite:
        Read a large block of data from the client,
        Large enough that it must be read in multiple passes
        When a chunk of data is recieved, send it back to the client
*/
int vchantests_bigwrite(libvchan_t *ctrl) {
    int sz = 0;
    char buf[VM_BIGWRITE_COMP_BUF_SIZE];
    int ret;

    libvchan_t *writeback = libvchan_server_init(0, VM_BIGWRITE_PORT, 0, 0);
    if(writeback != NULL)
        writeback = link_vchan_comp(writeback, &con);
    assert(writeback != NULL);

    ret = libvchan_recv(ctrl, &sz, sizeof(size_t));
    assert(ret > 0);

    DHELL("bigwrite: going to read %d bytes\n", sz);

    while(sz != 0) {
        ret = libvchan_read(ctrl, buf, VM_BIGWRITE_COMP_BUF_SIZE);
        assert(ret >= 0);
        sz -= ret;
        ret = libvchan_send(writeback, buf, ret);
    }

    DHELL("bigwrite: finished\n");

    return 0;
}

/*
    vchantests_packet_verify:
        Check if data in a test packet is correct
*/
static int vchantests_packet_verify(vchan_packet_t *pak) {
    for(int i = 0; i < 4; i++) {
        if(pak->datah[i] != i + pak->pnum) {
            /* Malformed data */
            return 0;
        }
    }
    return 1;
}

/*
    vchantests_packet:
        Send a number of packets to the test component, then wait for an ack
        test component checks for incorrect data
*/
int vchantests_packet(libvchan_t *ctrl) {
    size_t sz;
    char done = 1;
    int x, pnum;
    vchan_packet_t pak;

    libvchan_wait(ctrl);
    sz = libvchan_read(ctrl, &pnum, sizeof(int));
    assert(sz == sizeof(int));

    DHELL("number of packets to recieve = %d\n", pnum);

    for(x = 0; x < pnum; x++) {
        libvchan_wait(ctrl);
        /* Buffer sanity checking */
        assert(libvchan_data_ready(ctrl) != 0);
        assert(libvchan_buffer_space(ctrl) == FILE_DATAPORT_MAX_SIZE);
        /* Perform read operation */
        sz = libvchan_read(ctrl, &pak, sizeof(pak));
        /* See if the given packet is correct */
        assert(sz == sizeof(pak));
        assert(pak.pnum == x);
        assert(vchantests_packet_verify(&pak) == 1);
        assert(pak.guard == PACKET_TEST_GUARD);
        if(x % 500 == 0)
            DHELL("comp.packet %d|%d\n", x, sizeof(pak));
    }

    sz = libvchan_write(ctrl, &done, sizeof(char));
    assert(sz == sizeof(char));

    return 0;
}

/*
    CAmkES listening/test component
*/
int run(void) {
    int tcmd;
    int size;
    libvchan_t *ctrl;
    DHELL("ctrl initialisation\n");

    con.data_buf = (void *)share_mem;
    ctrl = libvchan_server_init(0, VCHANTESTS_VCHAN_PORT, 0, 0);
    if(ctrl != NULL)
        ctrl = link_vchan_comp(ctrl, &con);
    assert(ctrl != NULL);
    assert(vchantests_handshake(ctrl) == 0);

    /* Wait for test requests */
    while(1) {
        libvchan_wait(ctrl);
        size = libvchan_recv(ctrl, &tcmd, sizeof(int));
        if(size != sizeof(int)) {
            DHELL("error? %d bytes read from connection\n", FILE_DATAPORT_MAX_SIZE);
        } else {
            if(tcmd < 0 || tcmd > VCHANTESTS_MAX_TESTS) {
                DHELL("Invalid test:id .. ignoring %d:\n", tcmd);
                continue;
            }

            if(testop_table.tfunc[tcmd] == NULL) {
                DHELL("error! test:id %d: has no function attached, ignoring...\n", tcmd);
            } else {
                DHELL("Running test:id %d:\n", tcmd);
                (*testop_table.tfunc[tcmd])(ctrl);
            }
        }
    }
}
