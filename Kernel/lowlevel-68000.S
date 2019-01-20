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
	.globl	SYM (__umodsi3)
SYM (__umodsi3):
	movel	sp@(8), d1	/* d1 = divisor */
	movel	sp@(4), d0	/* d0 = dividend */
	movel	d1, sp@-
	movel	d0, sp@-
	jbsr	SYM (__udivsi3)
	addql	IMM (8), sp
	movel	sp@(8), d1	/* d1 = divisor */
#if !(defined(__mcf5200__) || defined(__mcoldfire__))
	movel	d1, sp@-
	movel	d0, sp@-
	jbsr	SYM (__mulsi3)	/* d0 = (a/b)*b */
	addql	IMM (8), sp
#else
	mulsl	d1,d0
#endif
	movel	sp@(4), d1	/* d1 = dividend */
	subl	d0, d1		/* d1 = a - (a/b)*b */
	movel	d1, d0
	rts


		.globl __hard_di,__hard_ei,__hard_irqrestore
		.globl doexec
		.globl get_usp, set_usp
		.globl outstring,outstringhex,outcharhex,outa0hex
		.globl bus_error,addr_error,illegal,divzero,chk,trapv,priv
		.globl trace,unimpa,unimpf,trap14,sysc_err
		.globl trap0,trap1,trap2,trap3,trap4,trap5,trap6,trap7
		.globl trap8,trap9,trap10,trap11,trap12,trap13,trap15
		.globl spurious,unexpected,uninit
		.globl udata_shadow
		.globl trap_via_signal
		.globl dump_registers
		.globl kernel_flag

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
__hard_di:
		move.w sr,d0
		and.w #$0700,d0
		or.w #$0700,sr
		rts

__hard_ei:
		and.w #$F8FF,sr
		rts

__hard_irqrestore:
		move.w sr,d0
		and.w #$F8FF,d0
		or.w 6(sp),d0
		move.w d0,sr
		rts

;
;	Put the supervisor stack back as if we had nothing on it (we just
;	longjmp'd out of the syscall path really). Then we push an exception
;	frame, clear the registers and rte. We need to rte as we want to
;	switch modes as we switch address so that we don't fault if there
;	is memory protection.
;
doexec:
		move.l 4(sp),a1			; go address
		lea.l 1024(a5),a7		; reset the supervisor stack

		and.w #$F8FF,sr			; IRQ on
		move.l a1,-(sp)			; return address
		clr.w -(sp)			; set up the status register
		move.l U_DATA__U_ISP(a5),a0
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


bus_error:	move.w sr,-(sp)
		move.w #2,-(sp)
		bra synchronous_exception
addr_error:	move.w sr,-(sp)
		move.w #3,-(sp)
		bra synchronous_exception
illegal:	move.w sr,-(sp)
		move.w #4,-(sp)
		bra synchronous_exception
divzero:	move.w sr,-(sp)
		move.w #5,-(sp)
		bra synchronous_exception
chk:		move.w sr,-(sp)
		move.w #6,-(sp)
		bra synchronous_exception
trapv:		move.w sr,-(sp)
		move.w #7,-(sp)
		bra synchronous_exception
priv:		move.w sr,-(sp)
		move.w #8,-(sp)
		bra synchronous_exception
trace:		move.w sr,-(sp)
		move.w #9,-(sp)
		bra synchronous_exception
unimpa:		move.w sr,-(sp)
		move.w #10,-(sp)
		bra synchronous_exception
unimpf:		move.w sr,-(sp)
		move.w #11,-(sp)
		bra synchronous_exception
trap0:		move.w sr,-(sp)
		move.w #32,-(sp)
		bra synchronous_exception
trap1:		move.w sr,-(sp)
		move.w #33,-(sp)
		bra synchronous_exception
trap2:		move.w sr,-(sp)
		move.w #34,-(sp)
		bra synchronous_exception
trap3:		move.w sr,-(sp)
		move.w #35,-(sp)
		bra synchronous_exception
trap4:		move.w sr,-(sp)
		move.w #36,-(sp)
		bra synchronous_exception
trap5:		move.w sr,-(sp)
		move.w #37,-(sp)
		bra synchronous_exception
trap6:		move.w sr,-(sp)
		move.w #38,-(sp)
		bra synchronous_exception
trap7:		move.w sr,-(sp)
		move.w #39,-(sp)
		bra synchronous_exception
trap8:		move.w sr,-(sp)
		move.w #40,-(sp)
		bra synchronous_exception
trap9:		move.w sr,-(sp)
		move.w #41,-(sp)
		bra synchronous_exception
trap10:		move.w sr,-(sp)
		move.w #42,-(sp)
		bra synchronous_exception
trap11:		move.w sr,-(sp)
		move.w #43,-(sp)
		bra synchronous_exception
trap12:		move.w sr,-(sp)
		move.w #44,-(sp)
		bra synchronous_exception
trap13:		move.w sr,-(sp)
		move.w #45,-(sp)
		bra synchronous_exception
trap15:		move.w sr,-(sp)
		move.w #47,-(sp)

;
;	Above us is the exception frame and return address
;	Add the registers to it
;
synchronous_exception:
		movem.l a0-a6/d0-d7,-(sp)
		move.l sp,-(sp)		; pointer to trap
		move.l udata_shadow,a5
		; Do the hard stuff in C
		bsr exception
		; This may not come back if the process dies or we panic
		addq #4,sp
		; Recover registers (may have changed)
		movem.l (sp)+,a0-a6/d0-d7
		addq #4,sp
		rte

;
;		Save as little as we can
;		on entry d0 = syscall, d1 = arg1 a0=arg2 a1=arg3 a2 = arg4
;		d0/d1 are our returns and we trash a0/a1 (a2 will be
;		preserved not that it matters much)
;		a5 we save, but otherwise we leave it to C to do needed
;		saves only
;
;		fork is special see the fork code.
;
trap14:
		move.l a5,-(sp)		; must be first to match fork
		move.l udata_shadow,a5
		move.b d0,U_DATA__U_CALLNO(a5)
		move.b #1,U_DATA__U_INSYS(a5)
		movem.l d1/a0-a2,U_DATA__U_ARGN(a5)
		move.l usp,a0
		move.l a0,U_DATA__U_SYSCALL_SP(a5)
		move.b #1,d0
		move.b d0,kernel_flag
		; FIXME: EI per platform really
		and.w #$F8FF,sr
		bsr unix_syscall
	        or.w #$0700,sr
		clr.b U_DATA__U_INSYS(a5)
		clr.b kernel_flag
		; Now we need to be careful. We can make a syscall in a
		; signal handler and we must not screw up the return from
		; this call
		move.w U_DATA__U_ERROR(a5),d1
		bne sysc_err
		move.l U_DATA__U_RETVAL(a5),d0
		; return data is now safe
sysc_sig:
		tst.b U_DATA__U_CURSIG(a5)
		bne return_via_signal
		; no signal - normal path
		move.l d0,a0		; need to set a0, so set to d0
					; avoids syscalls having to do it
					; for pointer returns
		move.l d0,a1		; no leakage please
		move.l (sp)+,a5
		rte
sysc_err:	moveq #-1,d0
		bra sysc_sig
;
;		Signal handler return path
;
;	FIXME: All the manipulation of usp relative data will need reworking
;	for any MMU based code as we don't know that usp is valid and we
;	need to update it's contents via user* helpers.
;
;
;	Return from a system call via a signal. Enter with only A5 on the
;	stack above the trap frame.
;
return_via_signal:
		; Stack now is return frame,a5
		move.w sr,-(sp)
		move.w #0,-(sp)
		movem.l a0-a6/d0-d7,-(sp)
		move.l sp,-(sp)
		move.l udata_shadow,a5
		bsr exception
		addq #4,sp
		movem.l (sp)+,a0-a6/d0-d7
		addq #4,sp
		move.l (sp)+,a5			; recover user A5
		rte				; into modified handler

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

/*
 *	Block copy helper
 *
 *	We use d0 = number of blocks
 *	       d1 = constant 32 for speeding up the maths
 *	       a0 = source
 *	       a1 = destination
 *	       d2-d7/a2-a6 	- copying registers (44 bytes a go)
 *
 */

copy_blocks:
		move.l 4(sp),a0
		move.l 8(sp),a1
		move.l 12(sp),d0

/* asm entry point */
copy_blocks_d0:
		movem.l d2-d7/a2-a6,-(sp)
copy_block512:
		movem.l (a0)+,d1-d7/a2-a6
		movem.l d1-d7/a2-a6,(a1)
		movem.l (a0)+,d1-d7/a2-a6
		movem.l d1-d7/a2-a6,48(a1)
		movem.l (a0)+,d1-d7/a2-a6
		movem.l d1-d7/a2-a6,96(a1)
		movem.l (a0)+,d1-d7/a2-a6
		movem.l d1-d7/a2-a6,144(a1)
		movem.l (a0)+,d1-d7/a2-a6
		movem.l d1-d7/a2-a6,192(a1)
		movem.l (a0)+,d1-d7/a2-a6
		movem.l d1-d7/a2-a6,248(a1)
		movem.l (a0)+,d1-d7/a2-a6
		movem.l d1-d7/a2-a6,288(a1)
		movem.l (a0)+,d1-d7/a2-a6
		movem.l d1-d7/a2-a6,336(a1)
		movem.l (a0)+,d1-d7/a2-a6
		movem.l d1-d7/a2-a6,384(a1)
		movem.l (a0)+,d1-d7/a2-a6
		movem.l d1-d7/a2-a6,432(a1)
		movem.l (a0)+,d1-d4
		movem.l d1-d4,480(a1)
		dbra d0,copy_block512
		movem.l (sp)+,d2-d7/a2-a6
		rts

clear_blocks:
		move.l 4(sp),a0
		move.l 8(sp),d0
clear_blocks_d0:
		movem.l d2-d7/a2-a6,-(sp)
		moveq #0,d1
		move.l d1,d2
		move.l d1,d3
		move.l d1,d4
		move.l d1,d5
		move.l d1,d6
		move.l d1,d7
		move.l d1,a1
		move.l d1,a2
		move.l d1,a3
		move.l d1,a4
		move.l d1,a5
		move.l d1,a6
clear512:
		/* End of the 512 byte block */
		lea 512(a0),a0
		/* zero in 52 byte chunks */
		movem.l d1-d7/a1-a6,-(a0)
		movem.l d1-d7/a1-a6,-(a0)
		movem.l d1-d7/a1-a6,-(a0)
		movem.l d1-d7/a1-a6,-(a0)
		movem.l d1-d7/a1-a6,-(a0)
		movem.l d1-d7/a1-a6,-(a0)
		movem.l d1-d7/a1-a6,-(a0)
		movem.l d1-d7/a1-a6,-(a0)
		movem.l d1-d7/a1-a6,-(a0)
		/* 9 * 52 + 44 */
		movem.l d1-d7/a1-a4,-(a0)
		/* Next block */
		lea 512(a0),a0
		dbra d0,clear512
		movem.l (sp)+,d2-d7/a2-a6
		rts

/*
 *	Debug. If we end up needing paged kernel these will need to be in
 *	common space
 */
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
		lsr.b #4,d0
		bsr hexdigit
		move.w (sp),d0
		and.b #15,d0
		bsr hexdigit
		move.w (sp)+,d0
		rts
/*
 *	General register dump logic
 */
dump_registers:
		movem.l a0-a2/d0-d2,-(sp)
		movem.l d0-d2,-(sp)
		bsr outa0hex
		move.b #' ',d0
		bsr outchar
		move.l a1,a0
		bsr outa0hex
		move.b #' ',d0
		bsr outchar
		move.l a2,a0
		bsr outa0hex
		move.b #' ',d0
		bsr outchar
		move.l a3,a0
		bsr outa0hex
		move.b #' ',d0
		bsr outchar
		move.l a4,a0
		bsr outa0hex
		move.b #' ',d0
		bsr outchar
		move.l a5,a0
		bsr outa0hex
		move.b #' ',d0
		bsr outchar
		move.l a6,a0
		bsr outa0hex
		move.b #' ',d0
		bsr outchar
		move.l a7,a0
		bsr outa0hex
		move.b #10,d0
		bsr outchar
		movem.l (sp)+,a0-a2
		bsr outa0hex
		move.b #' ',d0
		bsr outchar
		move.l a1,a0
		bsr outa0hex
		move.b #' ',d0
		bsr outchar
		move.l a2,a0
		bsr outa0hex
		move.b #' ',d0
		bsr outchar
		move.l d3,a0
		bsr outa0hex
		move.b #' ',d0
		bsr outchar
		move.l d4,a0
		bsr outa0hex
		move.b #' ',d0
		bsr outchar
		move.l d5,a0
		bsr outa0hex
		move.b #' ',d0
		bsr outchar
		move.l d6,a0
		bsr outa0hex
		move.b #' ',d0
		bsr outchar
		move.l d7,a0
		bsr outa0hex
		move.b #10,d0
		bsr outchar
		move.b #10,d0
		bsr outchar
		movem.l (sp)+,a0-a2/d0-d2
		rts
.area data
kernel_flag:	byte 0
udata_shadow:	long 0
trap_id:	word 0
