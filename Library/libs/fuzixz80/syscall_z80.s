		.export	__syscall

__syscall:
		call	__text
		ret	nc
		ld	(_errno), hl		; error path
		ld	hl, #0xffff
		ret
