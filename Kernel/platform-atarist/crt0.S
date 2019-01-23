#include "../kernel-68000.def"

/*
 *	Need to wipe BSS etc once we figure out our preferred boot method
 */
		.globl screenbase

.mri 1

start:
;
;		We run from TOS for the moment
;
		move.w #2,-(sp)
		trap #14
		move.l d0,screenbase
		clr.l (sp)
		move.w #32,-(sp)
		trap #1
;
;		Doesn't return but takes us stop
;
		addq.l	#6,sp
;
;		And on we go
;
		or #$0700,sr
		/* FIXME: hard coded ugly */
		move.l #uarea_block+UBLOCK_SIZE,a7
		bsr init_early
		bsr init_hardware
		bsr fuzix_main
		or #$0700,sr
stop:		bra stop
