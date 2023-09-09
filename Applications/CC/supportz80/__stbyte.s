;
;	Save byte from further off stack
;
		.export __stbyte
		.code

__stbyte:
		ex	(sp),hl		; tos is now value, hl is return
		ld	e,(hl)
		inc	hl
		ld	d,0
		ex	(sp),hl		; tos is now return hl is value
		ex	de,hl		; de is value hl is offset
		add	hl,sp
		ld	(hl),e
		ex	de,hl
		ret
