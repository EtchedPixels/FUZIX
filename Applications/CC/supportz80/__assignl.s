;
;	Assign the value in hireg:HL to lval at tos.
;
		.export __assignl
		.export	__assign0l
		.export	__assign0la
		.export	__assign1l
		.export __assignl0de

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

__assign1l:
		ld	a,1
		jr	__assign0la
; Assign 0L to lval in HL
__assign0l:
		xor	a
__assign0la:
		ld	(hl),a
		ld	e,a
		xor	a
		inc	hl
		ld	(hl),a
		inc	hl
		ld	(hl),a
		inc	hl
		ld	(hl),a
		ld	h,a
		ld	l,a
		ld	(__hireg),hl	; clear hireg
		pop	af
		ld	l,e
		ld	h,0
		ret

__assignl0de:
		xor	a
		ld	(hl),e
		inc	hl
		ld	(hl),d
		inc	hl
		ld	(hl),a
		inc	hl
		ld	(hl),a
		ex	de,hl		; return should be value
		ret
