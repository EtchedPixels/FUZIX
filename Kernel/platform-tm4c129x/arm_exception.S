/************************************************************************************
 *
 * Extracted from code
 *
 *   Copyright (C) 2009-2013, 2015-2016, 2018 Gregory Nutt. All rights reserved.
 *   Copyright (C) 2012 Michael Smith. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ************************************************************************************/

/************************************************************************************
 * Public Symbols
 ************************************************************************************/

	.globl		exception_common

	.syntax		unified
	.thumb
	.file		"arm_exception.S"

/************************************************************************************
 * Macro Definitions
 ************************************************************************************/

#define HW_XCPT_SIZE			32
#define SW_XCPT_SIZE			44
#define EXC_RETURN_PROCESS_STACK	0x04
#define REG_SP				0

/************************************************************************************
 * .text
 ************************************************************************************/

/* Common exception handling logic.  On entry here, the return stack is on either
 * the PSP or the MSP and looks like the following:
 *
 *      REG_XPSR
 *      REG_R15
 *      REG_R14
 *      REG_R12
 *      REG_R3
 *      REG_R2
 *      REG_R1
 * MSP->REG_R0
 *
 * And
 *      IPSR contains the IRQ number
 *      R14 Contains the EXC_RETURN value
 *      We are in handler mode and the current SP is the MSP
 *
 */

	.text
	.type	exception_common, function
	.thumb_func
exception_common:

	mrs		r0, ipsr				/* R0=exception number */

	/* Complete the context save */

	/* The EXC_RETURN value tells us whether the context is on the MSP or PSP */

	tst		r14, #EXC_RETURN_PROCESS_STACK /* nonzero if context on process stack */
	beq		1f						/* Branch if context already on the MSP */
	mrs		r1, psp					/* R1=The process stack pointer (PSP) */
	mov     sp, r1					/* Set the MSP to the PSP */

1:
	mov		r2, sp					/* R2=Copy of the main/process stack pointer */
	add		r2, #HW_XCPT_SIZE		/* R2=MSP/PSP before the interrupt was taken */
									/* (ignoring the xPSR[9] alignment bit) */
	mrs		r3, primask				/* R3=Current PRIMASK setting */


	stmdb	sp!, {r2-r11,r14}		/* Save the remaining registers plus the SP/PRIMASK values */

	/* There are two arguments to the exception
	 *
	 *   R0 = The IRQ number
	 *   R1 = The top of the stack points to the saved state
	 */

	mov		r1, sp

	/* Also save the top of the stack in a preserved register */

	mov		r4, sp

	/* Otherwise, we will re-use the interrupted thread's stack.  That may
	 * mean using either MSP or PSP stack for interrupt level processing (in
	 * kernel mode).
	 */

	bic		r2, r4, #7				/* Get the stack pointer with 8-byte alignment */
	mov		sp, r2					/* Instantiate the aligned stack */

	/* FUZIX: call hanlde_exception instead of arm_doirq */
	bl		handle_exception			/* R0=IRQ, R1=register save (msp) */
	mov		r1, r4					/* Recover R1=main stack pointer */

	/* On return from arm_doirq, R0 will hold a pointer to register context
	 * array to use for the interrupt return.  If that return value is the same
	 * as current stack pointer, then things are relatively easy.
	 */

	cmp		r0, r1					/* Context switch? */
	beq		2f						/* Branch if no context switch */

	/* We are returning with a pending context switch.  This case is different
	 * because in this case, the register save structure does not lie on the
	 * stack but, rather within a TCB structure.  We'll have to copy some
	 * values to the stack.
	 */

	/* Copy the hardware-saved context to the stack, and restore the software
	 * saved context directly.
	 *
	 * XXX In the normal case, it appears that this entire operation is unnecessary;
	 *     context switch time would be improved if we could work out when the stack
	 *     is dirty and avoid the work...
	 */

	add		r1, r0, #SW_XCPT_SIZE 	/* R1=Address of HW save area in reg array */
	ldmia	r1!, {r4-r11}			/* Fetch eight registers in HW save area */
	ldr		r1, [r0, #(4*REG_SP)]	/* R1=Value of SP before interrupt */
	stmdb	r1!, {r4-r11}			/* Store eight registers on the return stack */
	ldmia	r0!, {r2-r11,r14}		/* Recover R4-R11, r14 + 2 temp values */

	b		3f						/* Re-join common logic */

2:
	/* We are returning with no context switch.  We simply need to "unwind"
	 * the same stack frame that we created at entry.
	 */

	ldmia	r1!, {r2-r11,r14}		/* Recover R4-R11, r14 + 2 temp values */

3:
	/* The EXC_RETURN value tells us whether we are returning on the MSP or PSP
	 */

	tst		r14, #EXC_RETURN_PROCESS_STACK /* nonzero if context on process stack */
	ite		eq						/* next two instructions conditional */
	msreq	msp, r1					/* R1=The main stack pointer */
	msrne	psp, r1					/* R1=The process stack pointer */

	/* Restore the interrupt state */

	msr		primask, r3				/* Restore interrupts */

	/* Always return with R14 containing the special value that will: (1)
	 * return to thread mode, and (2) select the correct stack.
	 */

	bx		r14						/* And return */

	.size	exception_common, .-exception_common

	.end
