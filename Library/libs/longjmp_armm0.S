	.cpu cortex-m0
	.code 16
    .text
    .align 4
    .global longjmp
	.thumb_func

longjmp:
	/* Restore high regs. */

	add	r0, #4*4
	ldmia r0!, {r2, r3, r4, r5, r6}
	mov	r8, r2
	mov	r9, r3
	mov	r10, r4
	mov	fp, r5
	mov	sp, r6
	ldmia r0!, {r3} /* lr */

	/* Restore low regs.  */

	sub	r0, #10*4
	ldmia r0!, {r4, r5, r6, r7}

	/* Return val ? val : 1. */

	mov r0, r1
	bne	1f
	mov	r0, #1
1:
	bx r3
