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

#include <vchan/vmm_manager.h>
#include <vchan/vchan_copy.h>
#include <vchan/vchan_sharemem.h>
#include <vchan/libvchan.h>
#include <vchan/vchan_component.h>

#include <helloworld.h>

#include <camkes/dataport.h>

// #define SERVER_CORE_SIZE 4096
// static char core_buf[SERVER_CORE_SIZE];
// extern char *morecore_area;
// extern size_t morecore_size;

static char char_buf[256];

static void rec_packet(libvchan_t * con);
static void puffout_strings(libvchan_t * con);

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

static void rec_packet(libvchan_t * con) {
    size_t sz;
    int x;
    char comp[6];
    vchan_packet_t pak;
    for(x = 0; x < NUM_PACKETS * 2; x++) {
        sprintf(comp, "I%d", x);
        libvchan_wait(con);
        sz = libvchan_read(con, &pak, sizeof(pak));
        assert(sz == sizeof(pak));
        assert(strcmp(comp, pak.pnum) == 0);
        if(x % 100 == 0)
            printf("hello.packet %d\n", x);

    }

}

static void puffout_strings(libvchan_t * con) {
    size_t sz, len;
    vchan_header_t head;

    printf("hello: waiting for data\n");

    /* Wait for hello */
    libvchan_wait(con);

    sz = libvchan_read(con, &head, sizeof(head));
    assert(sz == sizeof(head));
    assert(head.msg_type == MSG_HELLO);
    head.msg_type = MSG_ACK;
    len = head.len;

    printf("hello: acking\n");

    /* Send off ack */
    sz = libvchan_write(con, &head, sizeof(head));
    assert(sz == sizeof(head));

    printf("hello: waiting for string\n");

    /* Read data */
    libvchan_wait(con);
    sz = libvchan_read(con, &char_buf, len);
    assert(sz == len);

    // head.msg_type = MSG_CONC;
    // sz = libvchan_write(con, &head, sizeof(head));
    // assert(sz == sizeof(head));
}

void pre_init(void) {
    // con.data_buf = (void *) share_mem;
    // morecore_area = core_buf;
    // morecore_size = SERVER_CORE_SIZE;
}

int run(void) {
    libvchan_t *connection;

    printf("Hello.Component Init\n");

    con.data_buf = (void *)share_mem;
    connection = libvchan_server_init(0, 25, 0, 0);
    if(connection != NULL)
        connection = link_vchan_comp(connection, &con);
    assert(connection != NULL);

    printf("Connection Active\n");

    printf("hello.handshake\n");
    puffout_strings(connection);
    printf("hello.packet\n");
    rec_packet(connection);

    printf("hello: indef wait\n");
    while(1);
}
