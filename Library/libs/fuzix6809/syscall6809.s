	.globl __syscall
	.globl _errno

	.area .text

__syscall:
	swi
	bne	error
	rts
error:
	std	_errno		; X is -1
	rts
