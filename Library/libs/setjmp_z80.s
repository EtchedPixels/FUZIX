;
;	Save IX IY and BC for register variables, the return address to use
;	and the stack pointer
;

		.export __setjmp
		.code

__setjmp:
	ld	hl,2
	add	hl,sp
	ld 	e,(hl)
	inc	hl
	ld	d,(hl)		; buffer into DE
	ld	hl,0
	add	hl,sp		; get SP
	ex	de,hl		; buffer is now HL
	ld	(hl),e
	inc	hl
	ld	(hl),d		; save SP
	inc	hl
	pop	de		; get return address
	push	de		; put return back
	ld 	(hl),e
	inc	hl
	ld	(hl),d
	inc 	hl		; now points after saved return
	ld	(hl),c		; save register variables BC
	inc	hl
	ld	(hl),b		;
	inc	hl
	push	ix
	pop	de
	ld	(hl),e		; save register variables IX
	inc	hl
	ld	(hl),d		;
	inc	hl
	push	iy
	pop	de
	ld	(hl),e		; save register variables IY
	inc	hl
	ld	(hl),d		;
	inc	hl
	ld	hl,0		; Return 0
	ret
