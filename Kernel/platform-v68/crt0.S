/*
 *	Need to wipe BSS etc once we figure out our preferred boot method
 *
 *	On entry we are loaded at $2000 with the loader and the loaders
 *	supervisor stack below us. We are in supervisor mode and the rest
 *	is our problem.
 */
		.globl _start
.mri 1

start:
		or #$0700,sr
		/* FIXME: hard coded ugly */
		move.l #uarea_block+508,a7
		bsr init_early
		bsr init_hardware
		bsr fuzix_main
		or #$0700,sr
stop:		bra stop
