			.export __ldwordw
			.setcpu 8085
			.code

__ldwordw:
	pop	d		; return address
	lhlx			; get the offset we want
	inx	d		; skip to the real return
	inx	d		; address
	push	d		; save it
	dad	sp		; turn offset into address
	xchg			; into d
	lhlx			; load contents
	ret
