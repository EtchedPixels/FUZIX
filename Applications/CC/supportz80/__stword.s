;
;	Save word from further off stack
;
		.export __stword
		.code

__stword:
		ex	(sp),hl		; tos is now value, hl is return
		ld	e,(hl)
		inc	hl
		ld	d,0
		ex	(sp),hl		; tos is now return hl is value
		ex	de,hl		; de is value hl is offset
		add	hl,sp
		ld	(hl),e
		inc	hl
		ld	(hl),d
		ex	de,hl		; value back into hl
		ret
