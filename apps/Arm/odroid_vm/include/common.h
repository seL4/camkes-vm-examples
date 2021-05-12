/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#ifndef COMMON_H
#define COMMON_H

/* Application IDs*/
#define CLIENT_APP_ID  0
#define CAN_APP_ID     1
#define NUM_APPS       2

/* Generic Constants */
#define TRUE 	1
#define FALSE	0

typedef enum{
	CLEAR 		= 2,
	ENABLE 		= 1,
	DISABLE 	= 0
}OPTION;

#define ROUND(a,b)		(((a) + (b) - 1) & ~((b) - 1))
#define roundup(x, y)		((((x) + ((y) - 1)) / (y)) * (y))

#endif
