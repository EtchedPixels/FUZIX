		.globl	__syscall
		.globl	_errno

__syscall:
		rst	#0x30
		ret	nc
		ld	(_errno), hl		; error path
		ld	hl, #0xffff
		ret
