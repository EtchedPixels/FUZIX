/*
 *	We do most of this in C on the 68000
 */
#include "kernel-68000.def"

.mri 0
/* libgcc1 routines for 68000 w/o floating-point hardware.
   Copyright (C) 1994, 1996, 1997, 1998 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

In addition to the permissions in the GNU General Public License, the
Free Software Foundation gives you unlimited permission to link the
compiled version of this file with other programs, and to distribute
those programs without any restriction coming from the use of this
file.  (The General Public License restrictions do apply in other
respects; for example, they cover modification of the file, and
distribution when not linked into another program.)

This file is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details. */

/* As a special exception, if you link this library with files
   compiled with GCC to produce an executable, this does not cause
   the resulting executable to be covered by the GNU General Public License.
   This exception does not however invalidate any other reasons why
   the executable file might be covered by the GNU General Public License.  */

/* Use this one for any 680x0; assumes no floating point hardware.
   The trailing " '" appearing on some lines is for ANSI preprocessors.  Yuk.
   Some of this code comes from MINIX, via the folks at ericsson.
   D. V. Henkel-Wallace (gumby@cygnus.com) Fete Bastille, 1992
*/

/* These are predefined by new versions of GNU cpp.  */

#ifndef __USER_LABEL_PREFIX__
#define __USER_LABEL_PREFIX__ _
#endif

#ifndef __REGISTER_PREFIX__
#define __REGISTER_PREFIX__
#endif

#ifndef __IMMEDIATE_PREFIX__
#define __IMMEDIATE_PREFIX__ #
#endif

/* ANSI concatenation macros.  */

#define CONCAT1(a, b) CONCAT2(a, b)
#define CONCAT2(a, b) a ## b

/* Use the right prefix for global labels.  */

#define SYM(x) CONCAT1 (__USER_LABEL_PREFIX__, x)

/* Use the right prefix for registers.  */

#define REG(x) CONCAT1 (__REGISTER_PREFIX__, x)

/* Use the right prefix for immediate values.  */

#define IMM(x) CONCAT1 (__IMMEDIATE_PREFIX__, x)

#define d0 REG (d0)
#define d1 REG (d1)
#define d2 REG (d2)
#define d3 REG (d3)
#define d4 REG (d4)
#define d5 REG (d5)
#define d6 REG (d6)
#define d7 REG (d7)
#define a0 REG (a0)
#define a1 REG (a1)
#define a2 REG (a2)
#define a3 REG (a3)
#define a4 REG (a4)
#define a5 REG (a5)
#define a6 REG (a6)
#define fp REG (fp)
#define sp REG (sp)

	.text
	.proc
	.globl	SYM (__mulsi3)
SYM (__mulsi3):
	movew	sp@(4), d0	/* x0 -> d0 */
	muluw	sp@(10), d0	/* x0*y1 */
	movew	sp@(6), d1	/* x1 -> d1 */
	muluw	sp@(8), d1	/* x1*y0 */
#if !(defined(__mcf5200__) || defined(__mcoldfire__))
	addw	d1, d0
#else
	addl	d1, d0
#endif
	swap	d0
	clrw	d0
	movew	sp@(6), d1	/* x1 -> d1 */
	muluw	sp@(10), d1	/* x1*y1 */
	addl	d1, d0

	rts

/* libgcc1 routines for 68000 w/o floating-point hardware.
   Copyright (C) 1994, 1996, 1997, 1998 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

In addition to the permissions in the GNU General Public License, the
Free Software Foundation gives you unlimited permission to link the
compiled version of this file with other programs, and to distribute
those programs without any restriction coming from the use of this
file.  (The General Public License restrictions do apply in other
respects; for example, they cover modification of the file, and
distribution when not linked into another program.)

This file is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details. */

/* As a special exception, if you link this library with files
   compiled with GCC to produce an executable, this does not cause
   the resulting executable to be covered by the GNU General Public License.
   This exception does not however invalidate any other reasons why
   the executable file might be covered by the GNU General Public License.  */

/* Use this one for any 680x0; assumes no floating point hardware.
   The trailing " '" appearing on some lines is for ANSI preprocessors.  Yuk.
   Some of this code comes from MINIX, via the folks at ericsson.
   D. V. Henkel-Wallace (gumby@cygnus.com) Fete Bastille, 1992
*/

/* These are predefined by new versions of GNU cpp.  */

#ifndef __USER_LABEL_PREFIX__
#define __USER_LABEL_PREFIX__ _
#endif

#ifndef __REGISTER_PREFIX__
#define __REGISTER_PREFIX__
#endif

#ifndef __IMMEDIATE_PREFIX__
#define __IMMEDIATE_PREFIX__ #
#endif

/* ANSI concatenation macros.  */

#define CONCAT1(a, b) CONCAT2(a, b)
#define CONCAT2(a, b) a ## b

/* Use the right prefix for global labels.  */

#define SYM(x) CONCAT1 (__USER_LABEL_PREFIX__, x)

/* Use the right prefix for registers.  */

#define REG(x) CONCAT1 (__REGISTER_PREFIX__, x)

/* Use the right prefix for immediate values.  */

#define IMM(x) CONCAT1 (__IMMEDIATE_PREFIX__, x)

#define d0 REG (d0)
#define d1 REG (d1)
#define d2 REG (d2)
#define d3 REG (d3)
#define d4 REG (d4)
#define d5 REG (d5)
#define d6 REG (d6)
#define d7 REG (d7)
#define a0 REG (a0)
#define a1 REG (a1)
#define a2 REG (a2)
#define a3 REG (a3)
#define a4 REG (a4)
#define a5 REG (a5)
#define a6 REG (a6)
#define fp REG (fp)
#define sp REG (sp)

	.text
	.proc
	.globl	SYM (__udivsi3)
SYM (__udivsi3):
#if !(defined(__mcf5200__) || defined(__mcoldfire__))
	movel	d2, sp@-
	movel	sp@(12), d1	/* d1 = divisor */
	movel	sp@(8), d0	/* d0 = dividend */

	cmpl	IMM (0x10000), d1 /* divisor >= 2 ^ 16 ?   */
	jcc	L3		/* then try next algorithm */
	movel	d0, d2
	clrw	d2
	swap	d2
	divu	d1, d2          /* high quotient in lower word */
	movew	d2, d0		/* save high quotient */
	swap	d0
	movew	sp@(10), d2	/* get low dividend + high rest */
	divu	d1, d2		/* low quotient */
	movew	d2, d0
	jra	L6

L3:	movel	d1, d2		/* use d2 as divisor backup */
L4:	lsrl	IMM (1), d1	/* shift divisor */
	lsrl	IMM (1), d0	/* shift dividend */
	cmpl	IMM (0x10000), d1 /* still divisor >= 2 ^ 16 ?  */
	jcc	L4
	divu	d1, d0		/* now we have 16 bit divisor */
	andl	IMM (0xffff), d0 /* mask out divisor, ignore remainder */

/* Multiply the 16 bit tentative quotient with the 32 bit divisor.  Because of
   the operand ranges, this might give a 33 bit product.  If this product is
   greater than the dividend, the tentative quotient was too large. */
	movel	d2, d1
	mulu	d0, d1		/* low part, 32 bits */
	swap	d2
	mulu	d0, d2		/* high part, at most 17 bits */
	swap	d2		/* align high part with low part */
	tstw	d2		/* high part 17 bits? */
	jne	L5		/* if 17 bits, quotient was too large */
	addl	d2, d1		/* add parts */
	jcs	L5		/* if sum is 33 bits, quotient was too large */
	cmpl	sp@(8), d1	/* compare the sum with the dividend */
	jls	L6		/* if sum > dividend, quotient was too large */
L5:	subql	IMM (1), d0	/* adjust quotient */

L6:	movel	sp@+, d2
	rts

#else /* __mcf5200__ || __mcoldfire__ */

/* Coldfire implementation of non-restoring division algorithm from
   Hennessy & Patterson, Appendix A. */
	link	a6,IMM (-12)
	moveml	d2-d4,sp@
	movel	a6@(8),d0
	movel	a6@(12),d1
	clrl	d2		| clear p
	moveq	IMM (31),d4
L1:	addl	d0,d0		| shift reg pair (p,a) one bit left
	addxl	d2,d2
	movl	d2,d3		| subtract b from p, store in tmp.
	subl	d1,d3
	jcs	L2		| if no carry,
	bset	IMM (0),d0	| set the low order bit of a to 1,
	movl	d3,d2		| and store tmp in p.
L2:	subql	IMM (1),d4
	jcc	L1
	moveml	sp@,d2-d4	| restore data registers
	unlk	a6		| and return
	rts
#endif /* __mcf5200__ || __mcoldfire__ */



		.globl di,ei,irqrestore
		.globl doexec
		.globl get_usp, set_usp
		.globl outstring,outstringhex,outcharhex,outa0hex
		.globl bus_error,addr_error,illegal,divzero,chk,trapv,priv
		.globl trace,unimpa,unimpf,misctrap,trap15,trap14,sysc_err
		.globl spurious,unexpected,uninit
		.globl udata_shadow

.mri 1
get_usp:	move.l	usp,a0
		rts

set_usp:	move.l 4(sp),a0
		move.l a0,usp
		rts

/*
 *	We don't currently have any fine grained IRQ control. We need to fix
 *	that!
 */
di:
		move.w sr,d0
		and.w #$0700,d0
		or.w #$0700,sr
		rts

ei:
		and.w #$F8FF,sr
		rts

irqrestore:
		move.w sr,d0
		and.w #$F8FF,d0
		or.w 4(sp),d0
		move.w d0,sr
		rts

doexec:
		;
		;	FIXME: we need to reset sp (we effectively longjmp
		; 	in supervisor space), then push the values. If we
		;	don't then the supervisor stack will go awry
		;
		move.l (sp)+,a0			; throw the return address
		clr.w -(sp)			; set up the status register
		move.l U_DATA__U_ISP,a0
		move.l a0,usp			; set the user stack
		moveq #0,d0			; wipe the registers
		move.l d0,d1
		move.l d0,d2
		move.l d0,d3
		move.l d0,d4
		move.l d0,d5
		move.l d0,d6
		move.l d0,d7
		move.l d0,a0
		move.l d0,a1
		move.l d0,a2
		move.l d0,a3
		move.l d0,a4
		move.l d0,a5
		move.l d0,a6
		rte		; hit user space (and will enable interrupts)


bus_error:	move.w #SIGBUS,-(sp)
		bra sig_or_die
addr_error:	move.w #SIGSEGV,-(sp)
		bra sig_or_die
illegal:	move.w #SIGILL,-(sp)
		bra sig_or_die
divzero:	move.w #SIGFPE,-(sp)
		bra sig_or_die
chk:		move.w #SIGFPE,-(sp)
		bra sig_or_die
trapv:		move.w #SIGFPE,-(sp)
		bra sig_or_die
priv:		move.w #SIGFPE,-(sp)
		bra sig_or_die
trace:		move.w #SIGTRAP,-(sp)
		bra sig_or_die
unimpa:		move.w #SIGFPE,-(sp)
		bra sig_or_die
unimpf:		move.w #SIGFPE,-(sp)
		bra sig_or_die
misctrap:	move.w #SIGILL,-(sp)
		bra sig_or_die
trap15:		move.w #SIGTRAP,-(sp)			; by convention
		bra sig_or_die
trap14:		movem.l a2-a6/d2-d7,-(sp)
		move.b #1,d0
		move.b d0,kernel_flag
		; FIXME: EI per platform really
		and.w #$F8FF,sr
		move.l udata_shadow,a5
		bsr unix_syscall
		; FIXME: signal handling in the C code or not ?
	        or.w #$0700,sr
		move.w U_DATA__U_ERROR,d1
		bne sysc_err
		move.w U_DATA__U_RETVAL,d0
		movem.l (sp)+,a2-a6/d2-d7
		rte
sysc_err:	move.w #-1,d0
		movem.l (sp)+,a2-a6/d2-d7
		rte

spurious:	movem.l a0-a6/d0-d7,-(sp)
		move.l #strspurious,a0
		bsr outstring
		movem.l (sp)+,a0-a6/d0-d7
		rte

unexpected:	movem.l a0-a6/d0-d7,-(sp)
		move.l #strunexpected,a0
		bsr outstring
unexpectedl:	bra unexpectedl

uninit:		movem.l a0-a6/d0-d7,-(sp)
		move.l #struninitialized,a0
		bsr outstring
		movem.l (sp)+,a0-a6/d0-d7
		rte

strspurious:	asciz "spurious interrupt\n"
struninitialized:
		asciz "uninitialized interrupt\n"
strunexpected:
		asciz "unexpected trap\n"

		.align 2
;
;	We do interrupts a bit differently - we use a single supervisor
; stack per process, not an interrupt stack. As a result our signal handling
; is a bit saner than 8bit.
;
#if 0
intvector1:	movem.l a0-a6/d0-d7,-(sp)
		bsr interrupt1
		movem.l (sp)+,a0-a6/d0-d7
		rte

intvector2:	movem.l a0-a6/d0-d7,-(sp)
		bsr interrupt2
		movem.l (sp)+,a0-a6/d0-d7
		rte

intvector3:	movem.l a0-a6/d0-d7,-(sp)
		bsr interrupt3
		movem.l (sp)+,a0-a6/d0-d7
		rte

intvector4:	movem.l a0-a6/d0-d7,-(sp)
		bsr interrupt4
		movem.l (sp)+,a0-a6/d0-d7
		rte

intvector5:	movem.l a0-a6/d0-d7,-(sp)
		bsr interrupt5
		movem.l (sp)+,a0-a6/d0-d7
		rte

intvector6:	movem.l a0-a6/d0-d7,-(sp)
		bsr interrupt6
		movem.l (sp)+,a0-a6/d0-d7
		rte

intvector7:	movem.l a0-a6/d0-d7,-(sp)
		bsr interrupt7
		movem.l (sp)+,a0-a6/d0-d7
		rte
#endif
sig_or_die:	
		tst.b kernel_flag
		beq sig_user
		move.b (sp),d0
		bsr outcharhex
		lea panic_sig,a0
		bsr outstring
		lea panic_sig2,a0
		bsr panic			; won't return
panic_sig:	ascii ": signal in kernel mode."
		byte 13,10
panic_sig2:	asciz ": ksig"
		.align 2

sig_user:	movem.l a0-a6/d0-d7,-(sp)
	        move.l udata_shadow,a5
		move.l U_DATA__U_PTAB(a5),-(sp)
		bsr ssig
		adda #4,sp
		; Do signal processing bit here if we return
		movem.l (sp)+,a0-a6/d0-d7
		rte

;
;	Debug
;
outstring:	move.b (a0)+,d0
		beq outstrend
		bsr outchar
		bra outstring
outstrend:	rts

outstringhex:	move.b (a0)+,d0
		beq outstrend
		bsr outcharhex
		bra outstringhex

hexdigit:
		add.b #48,d0
		cmp.b #58,d0
		bcs notltr
		add.b #7,d0
notltr:		bra outchar

outa0hex:	move.l a0,d0
		moveq #24,d1
		lsr.l d1,d0
		bsr outcharhex
		move.l a0,d0
		moveq #16,d1
		lsr.l d1,d0
		bsr outcharhex
		move.l a0,d0
		lsr.l #8,d0
		bsr outcharhex
		move.l a0,d0
outcharhex:	move.w d0,-(sp)
		lsr #4,d0
		bsr hexdigit
		move.w (sp),d0
		bsr hexdigit
		move.w (sp)+,d0
		rts

.area data
kernel_flag:	byte 0
udata_shadow:	long 0
