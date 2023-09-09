;
;		TOS = lval of object HL = amount
;
		.export __pluseq
		.code

__pluseq:
		ex	de,hl			; amount into D
		pop	hl		; return
		ex	(sp),hl		; swap with lval
		ex	de,hl		; get lval into D
		push	de		; save lval
		push	hl		; save value to add
		ex	de,hl
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		pop	hl		; get value back
		add	hl,de		; add __tmp to it
		ex	de,hl
		pop	hl		; get the TOS address
		ld	(hl),e
		inc	hl
		ld	(hl),d
		ex	de,hl
		ret
