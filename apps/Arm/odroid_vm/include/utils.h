/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
/*
 * utils.h
 *
 *  Created on: Aug 6, 2013
 *      Author: jxie
 */

#ifndef UTILS_H_
#define UTILS_H_


/**
 * Frequently used helper functions.
 */
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * Read from 32-bit register.
 */
inline uint32_t readl(const void *address)
{
	//printf("reading @ %x\n", address);
	return *((volatile uint32_t *)(address));
}

/**
 * Write to 32-bit register.
 */
inline void writel(uint32_t value, const void *address)
{
	//printf("writing %x @ %x\n", value, address);
	*((volatile uint32_t *)(address)) = value;
}

inline void spinlock_lock(int *lock)
{
	while (!__sync_bool_compare_and_swap(lock, 0, 1));
}

inline void spinlock_unlock(int *lock)
{
	while (!__sync_bool_compare_and_swap(lock, 1, 0));
}

inline void barrier(void)
{
	__sync_synchronize();
}

static inline uint32_t
__be32_to_cpu(uint32_t x){
    int i;
    uint32_t ret;
    char* a = (char*)&x;
    char* b = (char*)&ret;
    for(i = 0; i < sizeof(x); i++){
        b[i] = a[sizeof(x) - i - 1];
    }
    return ret;
}

/* A guess based on a 800MHz memory clock */
static __inline__ void udelay(uint64_t usec)
{
    volatile static int x;
    while (usec--) {
        for (x = 0; x < 4000; x++)
            ;
    }
}

static __inline__ int min(int a, int b)
{
    return a > b ? b : a;
}

static __inline__ void _acquire_spin_lock(volatile bool* lock){
	while(!__sync_bool_compare_and_swap(lock,0,1)){
		//seL4_Yield();
	}
}

static __inline__ void _release_spin_lock(volatile bool* lock){
	//release lock
	//*lock = 0;
	__sync_bool_compare_and_swap(lock,1,0);
}

static __inline__ void clrbits_le32(uint32_t * reg, uint32_t mask){
	*reg = *reg & (~mask);
}

static __inline__ void setbits_le32(uint32_t * reg, uint32_t mask){
	*reg = *reg | mask;
}

static __inline__ void clrsetbits_le32(uint32_t *reg, uint32_t clrm, uint32_t setm){
	*reg = ((*reg) & (~clrm)) | (setm);
}


/*
 * General Purpose Utilities
 */
#define min(X, Y)				\
	({ typeof(X) __x = (X);			\
		typeof(Y) __y = (Y);		\
		(__x < __y) ? __x : __y; })

#define max(X, Y)				\
	({ typeof(X) __x = (X);			\
		typeof(Y) __y = (Y);		\
		(__x > __y) ? __x : __y; })

#define min3(X, Y, Z)				\
	({ typeof(X) __x = (X);			\
		typeof(Y) __y = (Y);		\
		typeof(Z) __z = (Z);		\
		__x < __y ? (__x < __z ? __x : __z) :	\
		(__y < __z ? __y : __z); })

#define max3(X, Y, Z)				\
	({ typeof(X) __x = (X);			\
		typeof(Y) __y = (Y);		\
		typeof(Z) __z = (Z);		\
		__x > __y ? (__x > __z ? __x : __z) :	\
		(__y > __z ? __y : __z); })

#define MIN3(x, y, z)  min3(x, y, z)
#define MAX3(x, y, z)  max3(x, y, z)

/*
 * Return the absolute value of a number.
 *
 * This handles unsigned and signed longs, ints, shorts and chars.  For all
 * input types abs() returns a signed long.
 *
 */
#define abs(x) ({						\
		long ret;					\
		if (sizeof(x) == sizeof(long)) {		\
			long __x = (x);				\
			ret = (__x < 0) ? -__x : __x;		\
		} else {					\
			int __x = (x);				\
			ret = (__x < 0) ? -__x : __x;		\
		}						\
		ret;						\
	})

#endif /* UTILS_H_ */
