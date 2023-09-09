;
;	Assign the value in hireg:HL to lval at tos.
;
		.export __assignl
		.export	__assign0l
		.code

__assignl:
		ex	de,hl		; hireg:de is our value
		pop	hl
		ex	(sp),hl		; hl is now our pointer
		ld	(hl),e
		inc	hl
		ld	(hl),d
		inc	hl
		push	de		; for return
		ld	de,(__hireg)
		; de is now the high bytes
		ld	(hl),e
		inc	hl
		ld	(hl),d
		pop	hl		; saved low word
		ret


; Assign 0L to lval in HL
__assign0l:
		xor	a
		ld	(hl),a
		inc	hl
		ld	(hl),a
		inc	hl
		ld	(hl),a
		inc	hl
		ld	(hl),a
		ld	h,a
		ld	l,a
		ld	(__hireg),hl	; clear hireg
		ret
