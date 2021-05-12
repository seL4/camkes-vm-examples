/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <camkes.h>
#include <utils/util.h>
#include "commsec.h"

/* the raw payload is 80 byte mavlink packets, these are wrapped
 * around the commsec protocal */
#define MAVLINK_LEN 80
#define COMMSEC_PAYLOAD_LEN (securePkg_size_of_package(80))

/* our encryption keys. We only actually decrypt in one direction (RX)
 * so the key for transmit can be a dummy */
static uint8_t base_to_uav_key[16] = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0}; //RX
static uint8_t uav_to_base_key[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}; //TX
static uint32_t b2uSalt = 9219834;
static uint32_t u2bSalt = 284920;
/* state for decryption */
static commsec_ctx decrypt_state;

static void init_decrypt() {
    /* just need to init the state with the keys and salts */
    securePkg_init(&decrypt_state, 0, b2uSalt, base_to_uav_key,
                   u2bSalt, uav_to_base_key);
}

/* returns 1 (true) if packet was decrypted successfully, 0 on failure */
static int try_decrypt(char *buf, int msgsize) {
    /* our messages contain a commsec payload around a 2 byte frame, ensure we have this much */
    if (msgsize < 2 + COMMSEC_PAYLOAD_LEN) {
        return 0;
    }
    /* expect frame header of 0x7e first */
    if (buf[0] != 0x7e) {
        return 0;
    }
    /* tag after frame header should be 0 for air-data messages */
    if (buf[1] != 0) {
        return 0;
    }
    /* attempt to decrypt the actual frame to a temporary buffer */
    uint8_t temp[COMMSEC_PAYLOAD_LEN];
    /* skip the first two bytes that make up the frame header */
    memcpy(temp, buf + 2, COMMSEC_PAYLOAD_LEN);
    uint32_t ret;
    ret = securePkg_dec(&decrypt_state, temp, COMMSEC_PAYLOAD_LEN);
    if (ret != 0) {
        return 0;
    }
    /* success */
    return 1;
}

struct packet {
	char *data;
	uint32_t len;
};

static struct packet pkt_gcs;   //packet to be sent to GCS
static struct packet pkt_px4;   //packet to be sent to UAV
static struct packet check_buf; //CMD received from GCS

/* Max number of errors */
#define MAX_ERR  20
static uint32_t err_cnt = 0;    //Error counter

/* CAmkES initialization */
#define CHK_BUF_SIZE  256
void pre_init(void)
{
	pkt_gcs.data = (char*)gcs_buf;
	pkt_gcs.len = 0;

	pkt_px4.data = (char*)px4_buf;
	pkt_px4.len = 0;

	check_buf.data = malloc(CHK_BUF_SIZE);
	assert(check_buf.data);
	check_buf.len = 0;

	init_decrypt();
}

/* For debug */
static void UNUSED print_pkt(struct packet *pkt, int uart)
{
	printf("From %d: ", uart);
	for (int i = 0; i < pkt->len; i++) {
		if (i % 16 == 0) {
			printf("\n");
		}
		printf("%02X ", pkt->data[i]);
	}
	printf("\n");
}

/* Decrypt packet */
static int UNUSED check_packet(struct packet *pkt, char c)
{
	int ret;

	pkt->data[pkt->len++] = c;

	/* The packet is complete. */
	if (c == 0x7e && pkt->len > 1) {
		//print_pkt(pkt, 1);
		ret = try_decrypt(pkt->data, pkt->len);
		if (ret) {
			printf("True...\n");
		} else {
			printf("Fail...\n");
			err_cnt++;
		}
		pkt->len = 0;
	}
	return 0;
}

#define TRIGGER_LVL  20  //The GCS is expecting a very low latency.
void mavlink_recv(int uart_num, int c)
{
	struct packet *pkt = NULL;
	int(*func)(int,int) = NULL;

	switch (uart_num) {
		case 1:
			pkt = &pkt_px4;
			func = uart_px4_write;

			/* Put data from GCS into the buffer for checking. */
	//		check_packet(&check_buf, c);
			break;
		case 3:
			pkt = &pkt_gcs;
			func = uart_gcs_write;
			break;
		default:
			pkt = NULL;
			break;
	}

	pkt->data[pkt->len++] = c;
	if (pkt->len == TRIGGER_LVL) {
		func(0, pkt->len);
		pkt->len = 0;
		signal_emit();
	}

	/* XXX: Borrow the unused error counter to trigger the VM restart. */
	err_cnt++;
	if (err_cnt > MAX_ERR) {
		printf("We are under attack!!!\n");
		restart_vm_emit();
		err_cnt = 0;
	}
}

