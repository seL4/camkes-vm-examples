/*
 * Copyright 2014, NICTA
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(NICTA_BSD)
 */

/* standard */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* application common */
#include "can_inf.h"
#include "spi_inf.h"
#include "common.h"


int run(void)
{
	int error = 0;
	struct can_id can_id;
	struct can_frame tx, rx;

	/* Initialize CAN controller. */
	printf("Start CAN Loop-back Test\n");
	can_setup(125000);

	can_id.id = 0xF;
//	can_set_filter(can_id, 0xF);

	/* Prepare CAN frame. */
	tx.ident.id = 0x123;
	tx.ident.exide = 0;
	tx.ident.rtr = 0;
	tx.dlc = 8;
	tx.data[0] = 0x08;
	tx.data[1] = 0x07;
	tx.data[2] = 0x06;
	tx.data[3] = 0x05;
	tx.data[4] = 0x04;
	tx.data[5] = 0x03;
	tx.data[6] = 0x02;
	tx.data[7] = 0x01;

	while (1) {
		/* Send message */
		can_send(tx);
		printf("Send: error(%d), id(%x), data(%x, %x, %x, %x, %x, %x, %x, %x)\n",
			error, tx.ident.id,
			tx.data[0], tx.data[1], tx.data[2], tx.data[3],
			tx.data[4], tx.data[5], tx.data[6], tx.data[7]);

		tx.ident.id++;

		/* Receive message */
		can_recv(&rx);
		printf("Recv: error(%d), id(%x), data(%x, %x, %x, %x, %x, %x, %x, %x)\n",
			error, rx.ident.id,
			rx.data[0], rx.data[1], rx.data[2], rx.data[3],
			rx.data[4], rx.data[5], rx.data[6], rx.data[7]);
	}

	return 0;
}

