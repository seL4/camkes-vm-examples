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
#ifndef _VCHAN_TESTSUITE_H_
#define _VCHAN_TESTSUITE_H_

/*
	Vchan Testsuite
		Runs a set of tests designed to catch vchan bugs and incorrect behaviour

        All tests are assumed to be talking to a camkes component (helloworld),
            component acts according to protocol defined by a test

		To add a new test to the testsuite:

		1. Add the numeric value of the test and the name of the test into testsuite_mapping.h
			- Numeric value is usually the lowest unused number not currently used by a test
			- Modify the number of VCHAN_TESTSUITE_MAX_TESTS if applicable
		2. Add a function running the test into sel4_vchan_testsuite.c,
			add the function into the testsuite function table in vchan_testsuite.h
		3. Add a human readable name for the test and the tests numeric value into testsuite_mapping.txt
		4. Add testing code into the helloworld component
			- Helloworld component uses vchan_testsuite.h, but implements its own tests in hello.c (atm)
*/
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>

struct libvchan;
typedef struct libvchan libvchan_t;

#define VCHANTESTS_DEBUG_LVL        0

#define VCHANTESTS_VCHAN_PORT       25
#define VCHANTESTS_MAX_TESTS        20

#define PACKET_TEST                 0
#define FTP_TEST                    1
#define FUNNEL_TEST                 2
#define BADCALLS_TEST               3
#define CLOSE_REOPEN                4
#define PROD_CONS_TEST              5
#define EXTENDED_READWRITE          6
#define VM_BURST_TEST               7
#define BIGWRITE_TEST               8
#define CLOSE_TEST                  9

#define VCHAN_TESTSUITE_CLOSED      0xdffdffff
#define VCHANTESTS_HANDSHAKE_CODE   0xdeadbffe
#define VCHANTESTS_HANDSHAKE_ACK    0xdbffedee

#define TERROR(...) do{ \
		printf("vchantests-ERROR:%s:%d | ", __func__, __LINE__); printf(__VA_ARGS__); \
	} while(0);

#define TDPRINTF(lvl, ...) \
	do{ if(lvl >= 1 && lvl <= VCHANTESTS_DEBUG_LVL){ \
    	printf("vchantests:%d: %s:%d | ", lvl, __func__, __LINE__); \
    	printf(__VA_ARGS__); \
    } }while(0) \
        /*  */

/* Simple sanity test run before any tests */
int vchantests_handshake(libvchan_t *ctrl);
int verify_ctrl_correctness(libvchan_t *ctrl);

/*
    Test defines, function prototypes and function table
*/
int vchantests_packet(libvchan_t *ctrl);
int vchantests_prod_cons(libvchan_t *ctrl);
int vchantests_funnel(libvchan_t *ctrl);
int vchantests_vm_burst(libvchan_t *ctrl);
int vchantests_bigwrite(libvchan_t *ctrl);
int vchantests_close_reopen(libvchan_t *ctrl);
int vchantests_close(libvchan_t *ctrl);

/* vchantests_packet defined */
#define PACKET_TEST_GUARD    0xBEEDEADA
#define PACKET_TEST_NUM_PACKETS 2000

typedef struct vchan_header {
    int msg_type;
    int len;
} vchan_header_t;

typedef struct vchan_packet {
    int pnum;
    int datah[4];
    int guard;
} vchan_packet_t;

/* vchantests_prod_cons defines */
#define PROD_CONS_CHUNK 100
#define PROD_CONS_NUM 30

/* vchantests_vm_burst_test defines */
#define VM_BURST_SLEEP_TIME 2
#define VM_BURST_NUM_SLEEPS 4
#define VM_BURST_NUM_SENDS  4

#define VM_BURST_CHUNK_INTS 5012
#define VM_BURST_VM_SZ 430
#define VM_BURST_TOTAL_SZ (5012 * sizeof(int))

#define VM_BURST_CHECKSUM 0xffffffbb

const int vm_burst_base_nums[] = { 0xdbffedee, 0xffddee11, 0xdeadbeef, 0xbeeddead };

/* Set to whatever suitably big ASCII text file you want bigwrite to use */
const char bigwrite_file[]     = "war_and_peace.txt";
const char bigwrite_out[]  = "readback.txt";
const int bigwrite_cmd_size = sizeof("diff") + sizeof(bigwrite_file) + sizeof(bigwrite_out) + 4;

/* vchantests_bigwrite defines */
#define VM_BIGWRITE_COMP_BUF_SIZE 430
#define VM_BIGWRITE_PORT 128

/* - Function table - */
static struct test_ops {
    int (*tfunc[VCHANTESTS_MAX_TESTS])(libvchan_t *ctrl);
} testop_table = {
    .tfunc[PACKET_TEST]         =       &vchantests_packet,
    .tfunc[PROD_CONS_TEST]      =       &vchantests_prod_cons,
    .tfunc[VM_BURST_TEST]       =       &vchantests_vm_burst,
    .tfunc[FUNNEL_TEST]         =       &vchantests_funnel,
    .tfunc[BIGWRITE_TEST]       =       &vchantests_bigwrite,
    .tfunc[CLOSE_REOPEN]        =       &vchantests_close_reopen,
    .tfunc[CLOSE_TEST]        =         &vchantests_close,
};


#endif
