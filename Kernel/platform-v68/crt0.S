/*
 *	Need to wipe BSS etc once we figure out our preferred boot method
 *
 *	On entry we are loaded at $2000 with the loader and the loaders
 *	supervisor stack below us. We are in supervisor mode and the rest
 *	is our problem.
 */
		.globl __end
		.globl __bss_start

.mri 1
		byte $15
		byte $05
		byte $C0
		byte $DE
start:
		or #$0700,sr
		move.l #__bss_start,a0
		move.l #__end,d0
		sub.l a0,d0
		lsr.l #2,d0
wipebss:
		clr.l (a0)+
		dbra d0,wipebss

		/* FIXME: hard coded ugly */
		move.l #udata_block+1016,a7
		/* udata global */
		move.l #udata_block,a5
		bsr init_early
		bsr init_hardware
		bsr fuzix_main
		or #$0700,sr
stop:		bra stop
