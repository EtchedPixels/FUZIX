		.globl	__syscall
		.globl	_errno

__syscall:
		ex	(sp), hl		; hl is now return addr
						; stack is syscall
		ex	de, hl			; save return addr in de
		call	#0xFFF7			; magic number
		ex	de, hl			; undo the magic
		ex	(sp), hl
		ex	de, hl			; return with HL
		ret	nc			; ok
		ld	(_errno), hl		; error path
		ld	hl, #0xffff
		ret
