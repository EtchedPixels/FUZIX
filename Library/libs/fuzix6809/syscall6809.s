	.globl __syscall
	.globl __syscall_mangled
	.globl _errno

	.area .text

__syscall:
	swi
	bne	error
	rts
error:
	std	_errno		; X is -1
	rts

; for variadic functions:
; compensate for the 1st argument that we removed
__syscall_mangled:
	swi
	beq	noerr
	std	_errno
noerr:
	puls d		; get return address
	pshs d,x	; inject a word on stack
	rts
