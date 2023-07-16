/* Miscellaneous functions imported from libgcc.

   Copyright (C) 2003-2017 Free Software Foundation, Inc.
   Contributed by CodeSourcery, LLC.

   This file is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 3, or (at your option) any
   later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.

   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* ANSI concatenation macros.  */

#define CONCAT1(a, b) CONCAT2(a, b)
#define CONCAT2(a, b) a ## b

/* Use the right prefix for global labels.  */

#define SYM(x) CONCAT1 (__USER_LABEL_PREFIX__, x)

#ifdef __ELF__
#ifdef __thumb__
#define __PLT__  /* Not supported in Thumb assembler (for now).  */
#else
#error "Not __thumb__"
#endif
#define TYPE(x) .type SYM(x),function
#define SIZE(x) .size SYM(x), . - SYM(x)
#define LSYM(x) .x
#else
#error "Not __ELF__"
#endif


#ifdef __thumb__
#define THUMB_FUNC .thumb_func
#define THUMB_CODE .force_thumb
# if defined(__thumb2__)
#define THUMB_SYNTAX .syntax divided
# else
#error "Not __thumb2__!"
# endif
#else
#error "Not __thumb__!"
#endif

.macro FUNC_START name sp_section=
  .ifc \sp_section, function_section
	.section	.text.__\name,"ax",%progbits
  .else
	.text
  .endif
	.globl SYM (__\name)
	TYPE (__\name)
	.align 0
	THUMB_CODE
	THUMB_FUNC
	THUMB_SYNTAX
SYM (__\name):
.endm

#if defined(__thumb2__)

/* For Thumb-2 we build everything in thumb mode.  */
.macro ARM_FUNC_START name sp_section=
       FUNC_START \name \sp_section
       .syntax unified
.endm
#define EQUIV .thumb_set
.macro  ARM_CALL name
	bl	__\name
.endm

#else

#error "Not __thumb2__"

#endif

/* The Unified assembly syntax allows the same code to be assembled for both
   ARM and Thumb-2.  However this is only supported by recent gas, so define
   a set of macros to allow ARM code on older assemblers.  */
#if defined(__thumb2__)
.macro do_it cond, suffix=""
	it\suffix	\cond
.endm
.macro shift1 op, arg0, arg1, arg2
	\op	\arg0, \arg1, \arg2
.endm
#define do_push	push
#define do_pop	pop
#define COND(op1, op2, cond) op1 ## op2 ## cond
/* Perform an arithmetic operation with a variable shift operand.  This
   requires two instructions and a scratch register on Thumb-2.  */
.macro shiftop name, dest, src1, src2, shiftop, shiftreg, tmp
	\shiftop \tmp, \src2, \shiftreg
	\name \dest, \src1, \tmp
.endm
#else

#error "Not __thumb2__"

#endif

#if defined(__ARM_ARCH_2__)
# define __ARM_ARCH__ 2
#endif

#if defined(__ARM_ARCH_3__)
# define __ARM_ARCH__ 3
#endif

#if defined(__ARM_ARCH_3M__) || defined(__ARM_ARCH_4__) \
	|| defined(__ARM_ARCH_4T__)
/* We use __ARM_ARCH__ set to 4 here, but in reality it's any processor with
   long multiply instructions.  That includes v3M.  */
# define __ARM_ARCH__ 4
#endif

#if defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5T__) \
	|| defined(__ARM_ARCH_5E__) || defined(__ARM_ARCH_5TE__) \
	|| defined(__ARM_ARCH_5TEJ__)
# define __ARM_ARCH__ 5
#endif

#if defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) \
	|| defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) \
	|| defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_6T2__) \
	|| defined(__ARM_ARCH_6M__)
# define __ARM_ARCH__ 6
#endif

#if defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) \
	|| defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) \
	|| defined(__ARM_ARCH_7EM__)
# define __ARM_ARCH__ 7
#endif

#if defined(__ARM_ARCH_8A__) || defined(__ARM_ARCH_8M_BASE__) \
	|| defined(__ARM_ARCH_8M_MAIN__)
# define __ARM_ARCH__ 8
#endif

#ifndef __ARM_ARCH__
#error "Unable to determine architecture."
#endif

#if (__ARM_ARCH__ > 4) || defined(__ARM_ARCH_4T__)

# define RET		bx	lr
# define RETc(x)	bx##x	lr

#else

#error "Not __ARM_ARCH__ > 4"

#endif

/* we can use STRD/LDRD on v5TE and later, and any Thumb-2 architecture. */
#if (defined(__ARM_EABI__)                                            \
     && (defined(__thumb2__)                                          \
         || (__ARM_ARCH >= 5 && defined(__TARGET_FEATURE_DSP))))
#define CAN_USE_LDRD 1
#else
#error "Cannot use STRD/LDRD"
#endif

#ifdef __ARMEB__
#error "Not Little Endian"
#else
#define xxh r1
#define xxl r0
#define yyh r3
#define yyl r2
#endif

.macro test_div_by_zero signed
/* Tail-call to divide-by-zero handlers which may be overridden by the user,
   so unwinding works properly.  */
#if defined(__thumb2__)
	cbnz	yyh, 2f
	cbnz	yyl, 2f
	cmp	xxh, #0
	.ifc \signed, unsigned
	do_it	eq
	cmpeq	xxl, #0
	do_it	ne, t
	movne	xxh, #0xffffffff
	movne	xxl, #0xffffffff
	.else
	do_it	lt, tt
	movlt	xxl, #0
	movlt	xxh, #0x80000000
	blt	1f
	do_it	eq
	cmpeq	xxl, #0
	do_it	ne, t
	movne	xxh, #0x7fffffff
	movne	xxl, #0xffffffff
	.endif
1:	/* die on division by zero */
	wfi
	b	1b
2:
#else

#error "Not __thumb2__"

#endif
.endm

/* set up stack from for call to __udivmoddi4. At the end of the macro the
   stack is arranged as follows:
		sp+12	/ space for remainder
		sp+8	\ (written by __udivmoddi4)
		sp+4	lr
		sp+0	sp+8 [rp (remainder pointer) argument for __udivmoddi4]

 */
.macro push_for_divide fname
#if defined(__thumb2__) && CAN_USE_LDRD
	sub	ip, sp, #8
	strd	ip, lr, [sp, #-16]!
#else
	sub	sp, sp, #8
	do_push	{sp, lr}
#endif
	.cfi_adjust_cfa_offset 16
	.cfi_offset 14, -12
.endm

/* restore stack */
.macro pop_for_divide
	ldr	lr, [sp, #4]
#if CAN_USE_LDRD
	ldrd	r2, r3, [sp, #8]
	add	sp, sp, #16
#else
	add	sp, sp, #8
	do_pop	{r2, r3}
#endif
	.cfi_restore 14
	.cfi_adjust_cfa_offset 0
.endm

/* Perform 64 bit signed division.
   Inputs:
	r0:r1	numerator
	r2:r3	denominator
   Outputs:
	r0:r1	quotient
	r2:r3	remainder
 */
ARM_FUNC_START aeabi_uldivmod
	.cfi_startproc
	test_div_by_zero	unsigned

	push_for_divide	__aeabi_uldivmod
	/* arguments in (r0:r1), (r2:r3) and *sp */
	bl	SYM(__udivmoddi4) __PLT__
	pop_for_divide
	RET
	.cfi_endproc
