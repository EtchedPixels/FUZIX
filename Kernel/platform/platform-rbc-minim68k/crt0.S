/*
 *	On entry we are loaded at 0x1000 with the loader and possibly the
 *	M68k debugger (non-RETAIL == DEBUG mode)  The stack pointer is set
 *	to high memory.
 */
		#include "../../cpu-68000/kernel-68000.def"

		.globl __end
		.globl __bss_start

.mri 1
start:
		or #$0700,sr
		move.l #__bss_start,a0
		move.l #__end,d0
		sub.l a0,d0
		lsr.l #2,d0
wipebss:
		clr.l (a0)+
		dbra d0,wipebss

		lea udata_block+UBLOCK_SIZE,a7
		bsr init_early
		bsr init_hardware
		bsr fuzix_main
		or #$0700,sr
stop:		bra stop
