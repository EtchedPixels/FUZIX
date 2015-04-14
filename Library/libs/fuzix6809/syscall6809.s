	.globl __syscall
	.globl _errno

	.area .text

__syscall:
	swi
	bne	error
	rts
error:
	sta	_errno
	ldx	#0
	stx	_errno+1
	leax	-1,x		; Return $FFFF (-1)
	rts
