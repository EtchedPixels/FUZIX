		.globl	__syscall
		.globl __call_sys
		.globl	_errno

__syscall:
		call	__call_sys
		ret	nc
		ld	(_errno), hl		; error path
		ld	hl, #0xffff
		ret
