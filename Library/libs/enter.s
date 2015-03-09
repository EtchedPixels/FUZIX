;
;	These are used for space saving with the modified sdcc
;
	.globl __enter, __enter_s

__enter:
	pop hl		; return address
	push ix		; save frame pointer
	ld ix, #0
	add ix, sp	; set ix to the stack frame
	jp (hl)		; and return

__enter_s:
	pop hl		; return address
	push ix		; save frame pointer
	ld ix, #0
	add ix, sp	; frame pointer
	ld e, (hl)	; size byte
	ld d, #0xFF	; always minus something..
	inc hl
	ex de, hl
	add hl, sp
	ld sp, hl
	push de
	ret
