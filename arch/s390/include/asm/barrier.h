/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright IBM Corp. 1999, 2009
 *
 * Author(s): Martin Schwidefsky <schwidefsky@de.ibm.com>
 */

#ifndef __ASM_BARRIER_H
#define __ASM_BARRIER_H

#include <asm/alternative.h>

/*
 * Force strict CPU ordering.
 * And yes, this is required on UP too when we're talking
 * to devices.
 */

#ifdef CONFIG_HAVE_MARCH_Z196_FEATURES
/* Fast-BCR without checkpoint synchronization */
#define __ASM_BARRIER "bcr 14,0\n"
#else
#define __ASM_BARRIER "bcr 15,0\n"
#endif

#define mb() do {  asm volatile(__ASM_BARRIER : : : "memory"); } while (0)

/* Prevent speculative execution past this barrier. */
#define barrier_nospec()						\
	do {								\
		asm volatile(ALTERNATIVE("", ".long 0xb2e8f000", 81)	\
			     : : : "memory");				\
	} while (0)

#define rmb()				barrier()
#define wmb()				barrier()
#define dma_rmb()			mb()
#define dma_wmb()			mb()
#define __smp_mb()			mb()
#define __smp_rmb()			rmb()
#define __smp_wmb()			wmb()

<<<<<<< HEAD
#define read_barrier_depends()		do { } while (0)
#define smp_read_barrier_depends()	do { } while (0)

#define smp_mb__before_atomic()		smp_mb()
#define smp_mb__after_atomic()		smp_mb()

#define smp_store_mb(var, value)	do { WRITE_ONCE(var, value); smp_mb(); } while (0)

#define smp_store_release(p, v)						\
=======
#define __smp_store_release(p, v)					\
>>>>>>> temp
do {									\
	compiletime_assert_atomic_type(*p);				\
	barrier();							\
	WRITE_ONCE(*p, v);						\
} while (0)

#define __smp_load_acquire(p)						\
({									\
	typeof(*p) ___p1 = READ_ONCE(*p);				\
	compiletime_assert_atomic_type(*p);				\
	barrier();							\
	___p1;								\
})

<<<<<<< HEAD
=======
#define __smp_mb__before_atomic()	barrier()
#define __smp_mb__after_atomic()	barrier()

>>>>>>> temp
/**
 * array_index_mask_nospec - generate a mask for array_idx() that is
 * ~0UL when the bounds check succeeds and 0 otherwise
 * @index: array element index
 * @size: number of elements in array
 */
#define array_index_mask_nospec array_index_mask_nospec
static inline unsigned long array_index_mask_nospec(unsigned long index,
						    unsigned long size)
{
	unsigned long mask;

	if (__builtin_constant_p(size) && size > 0) {
		asm("	clgr	%2,%1\n"
		    "	slbgr	%0,%0\n"
		    :"=d" (mask) : "d" (size-1), "d" (index) :"cc");
		return mask;
	}
	asm("	clgr	%1,%2\n"
	    "	slbgr	%0,%0\n"
	    :"=d" (mask) : "d" (size), "d" (index) :"cc");
	return ~mask;
}

<<<<<<< HEAD
=======
#include <asm-generic/barrier.h>

>>>>>>> temp
#endif /* __ASM_BARRIER_H */
