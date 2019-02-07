		.globl	__syscall
		.globl	_errno

;
;	For Rabbit we use the new syscall ABI we plan to use for Z80.
;
__syscall:
		rst	0x28		; Needs to be an RST if we want
					; to also run on R3000A so we get
					; a system mode entry
		ret	nc		; ok
		ld	(_errno), hl	; error path
		ld	hl, #0xffff
		ret
