;
;	This uses the revised API that Z80 now uses.
;
		.code
		.export __syscall

__syscall:
		call __text			; syscall stub is at text
						; start, set by kernel
		; returns in HL
		rnc				; ok return in HL
		shld	_errno			; error path
		lxi	h,0xffff
		ret
