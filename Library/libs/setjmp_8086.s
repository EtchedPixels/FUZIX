/*
 *	Class C ABI, small or tiny model. Trying to deal with
 *	large model and all the fun and thunking joy is definitely
 *	for another time.
 */
	.text

	.globl setjmp
	.globl longjmp

setjmp:
	pop	%cx
	pop	%bx
	mov	%cx,0(%bx)
	mov	%sp,2(%bx)
	mov	%bp,4(%bx)
	mov	%si,6(%bx)
	mov	%di,8(%bx)
	mov	%es,10(%bx)
	/* Mend stack, return 0 */
	xor	%ax,%ax
	push	%bx
	jmp	*%cx

longjmp:
	pop	%cx	/* Recover PC, return, longjmp arg */
	pop	%bx
	pop	%ax
	cmpw	$0,%ax
	je	arg_sane
	incw	%ax
arg_sane:
	mov	0(%bx),%cx
	mov	4(%bx),%bp
	mov	6(%bx),%si
	mov	8(%bx),%di
	mov	10(%bx),%es
	/* Do SP last. This doesn't usually matter because the %bx frame
	   is required to be in scope. However if you have a register
	   global and take a signal it does */
	mov	2(%bx),%sp
	push	%bx	/* Caller expects an argument in the unwind */
	jmp	*%cx
