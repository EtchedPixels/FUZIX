	.cpu cortex-m0
	.code 16
    .text
    .align 4
    .global setjmp
	.thumb_func

setjmp:
	/* Save callee-saved low regs. */

	stmia r0!, {r4, r5, r6, r7}

	/* Save high regs. */

	mov	r1, r8
	mov	r2, r9
	mov	r3, r10
	mov	r4, fp
	mov	r5, sp
	mov	r6, lr
	stmia r0!, {r1, r2, r3, r4, r5, r6}
	sub	r0, #10*4

	/* Restore callee-saved low regs.  */

	ldmia r0!, {r4, r5, r6, r7}

	/* Return zero.  */
	mov	r0, #0
	bx lr
