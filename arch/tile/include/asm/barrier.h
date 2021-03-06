/*
 * Copyright 2010 Tilera Corporation. All Rights Reserved.
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation, version 2.
 *
 *   This program is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, GOOD TITLE or
 *   NON INFRINGEMENT.  See the GNU General Public License for
 *   more details.
 */

#ifndef _ASM_TILE_BARRIER_H
#define _ASM_TILE_BARRIER_H

#ifndef __ASSEMBLY__

#include <linux/types.h>
#include <arch/chip.h>
#include <arch/spr_def.h>
#include <asm/timex.h>

#define read_barrier_depends()	do { } while (0)

#define __sync()	__insn_mf()

#if !CHIP_HAS_MF_WAITS_FOR_VICTIMS()
#include <hv/syscall_public.h>
static inline void __mb_incoherent(void)
{
	long clobber_r10;
	asm volatile("swint2"
		     : "=R10" (clobber_r10)
		     : "R10" (HV_SYS_fence_incoherent)
		     : "r0", "r1", "r2", "r3", "r4",
		       "r5", "r6", "r7", "r8", "r9",
		       "r11", "r12", "r13", "r14",
		       "r15", "r16", "r17", "r18", "r19",
		       "r20", "r21", "r22", "r23", "r24",
		       "r25", "r26", "r27", "r28", "r29");
}
#endif

static inline void
mb_incoherent(void)
{
	__insn_mf();

#if !CHIP_HAS_MF_WAITS_FOR_VICTIMS()
	{
#if CHIP_HAS_TILE_WRITE_PENDING()
		const unsigned long WRITE_TIMEOUT_CYCLES = 400;
		unsigned long start = get_cycles_low();
		do {
			if (__insn_mfspr(SPR_TILE_WRITE_PENDING) == 0)
				return;
		} while ((get_cycles_low() - start) < WRITE_TIMEOUT_CYCLES);
#endif 
		(void) __mb_incoherent();
	}
#endif 
}

#define fast_wmb()	__sync()
#define fast_rmb()	__sync()
#define fast_mb()	__sync()
#define fast_iob()	mb_incoherent()

#define wmb()		fast_wmb()
#define rmb()		fast_rmb()
#define mb()		fast_mb()
#define iob()		fast_iob()

#ifdef CONFIG_SMP
#define smp_mb()	mb()
#define smp_rmb()	rmb()
#define smp_wmb()	wmb()
#define smp_read_barrier_depends()	read_barrier_depends()
#else
#define smp_mb()	barrier()
#define smp_rmb()	barrier()
#define smp_wmb()	barrier()
#define smp_read_barrier_depends()	do { } while (0)
#endif

#define set_mb(var, value) \
	do { var = value; mb(); } while (0)

#endif 
#endif 
