/* Dummy for now */
	.globl	_start
	.globl	environ
	.text

_start:
	movw	%sp,%bp
	movw	4(%bp),%ax
	movw	%ax,environ
	call	__stdio_init_vars
	call	main
	push	%ax
	call	exit
	/* Should never get here */
oops:	jmp	oops

	.data
environ:
	.word	0