#include "../kernel-68000.def"

/*
 *	Need to wipe BSS etc once we figure out our preferred boot method
 */
.mri 1

start:
;
;		Ensure interrupts are off
;
		or #$0700,sr
		/* FIXME: hard coded ugly */
		move.l #uarea_block+UBLOCK_SIZE,a7

		lea.l __bss_start,a0
		lea.l __end,a1

wipe_bss:	clr.l (a0)+
		cmp.l a0,a1
		bne wipe_bss

		bsr init_early
		bsr init_hardware
		bsr fuzix_main
		or #$0700,sr
stop:		bra stop
