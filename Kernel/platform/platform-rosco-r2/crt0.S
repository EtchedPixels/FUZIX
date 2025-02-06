/*
 *	Need to wipe BSS etc once we figure out our preferred boot method
 *
 *	On entry we are loaded at $40000. We copy ourselves down to $400.
 *	We are in supervisor mode and the rest is our problem.
 */
		#include "../../cpu-68000/kernel-68000.def"

		.globl start
		.globl start2
		.globl __end
		.globl __bss_start

.mri 1

start:
		/* Position-independent code to copy from start to
		 * __bss_start down to $400. Code borrowed from the
		 * Rosco loader code.
		 */
RELOC:                           /* position-independent load addr */
    lea.l   RELOC(PC),A0         /* PC-rel source addr (load addr) */
    move.l  #start,A1            /* absolute dest addr (run addr) */
    move.l  #__bss_start,D0      /* init section absolute end addr */
    sub.l   A1,D0                /* subtract dest addr for init length */
    lsr.l   #2,D0                /* convert to long words */
    subq.l  #1,D0                /* subtract 1 for dbra */
.INIT_LOOP:
    move.l  (A0)+,(A1)+          /* copy long word from source to dest */
    dbra    D0,.INIT_LOOP        /* loop until __bss_start */

    move.l  #start2,A0		 /* Use A0, otherwise the generated code */
				 /* is PC-relative which we don't want. */
    jmp	    (A0)	 	 /* Jump to the copied start2 code */

start2:
		or #$0700,sr
		move.l #__bss_start,a0
		move.l #__end,d0
		sub.l a0,d0
		lsr.l #2,d0
wipebss:
		clr.l (a0)+
		dbra d0,wipebss

		lea udata_block0+UBLOCK_SIZE,a7
		bsr init_early
		bsr init_hardware
		bsr fuzix_main
		or #$0700,sr
stop:		bra stop
