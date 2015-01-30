/*
 *	Need to wipe BSS etc once we figure out our preferred boot method
 */
		.globl _start

.mri 1

_start:
		or #$0700,sr
		/* FIXME: hard coded ugly */
		move.l uarea_block+508,a7
		jsr init_early
		jsr init_hardware
		jsr fuzix_main
		or #$0700,sr
stop:		bra stop
