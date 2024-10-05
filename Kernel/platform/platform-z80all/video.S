	.export _clear_across
	.export _clear_lines
	.export _cursor_disable
	.export _cursor_off
	.export _cursor_on
	.export _do_beep
	.export _plot_char
	.export _scroll_up
	.export _scroll_down
	.export _video_init
	.export _vtattr_notify

	.export _vtattr_cap

	.code

_video_init:
	push	bc
	ld	bc,0x000F
	ld	hl,_fontdata_8x8+767
	otdr
	dec	c
	otdr
	dec	c
	otdr
	pop	bc
	ret

addr:
;	E = X D = Y
;	return BC = port
	ld	c,d	; Port number - high bits
	ld	b,e	; Offset - low bits
	sla	b
	sla	b
	srl	c
	rr	b
	srl	c
	rr	b
	ret

_do_beep:
_cursor_disable:
	ret

_vtattr_notify:
	ld	a,(_vtattr)
	and	1		; turn bit 0, into 00 or FF
	dec	a
	cpl	
	ld	(inverse),a
	ret

_cursor_off:
	push	bc
	ld	bc,(cursor)
do_xor:
	in	a,(c)
	xor	0x80
	out	(c),a
	pop	bc
	ret

_cursor_on:
	ld	hl,2
	add	hl,sp
	push	bc
	ld	d,(hl)		; Y
	inc	hl
	inc	hl
	ld	e,(hl)		; X

	call	addr

	ld	(cursor),bc
	jr	do_xor

_plot_char:
	ld	hl,2
	add	hl,sp
	push	bc
	ld	d,(hl)
	inc	hl
	inc	hl
	ld	e,(hl)
	inc	hl
	inc	hl

	call	addr

	ld	a,(inverse)
	xor	(hl)
	; TODO inverse
	out	(c),a
	pop	bc
	ret

_clear_across:
	ld	hl,2
	add	hl,sp
	push	bc
	ld	d,(hl)
	inc	hl
	inc	hl
	ld	e,(hl)
	inc	hl
	inc	hl

	call	addr

	ld	a,(hl)
	or	a
	jr	z, nobytes

	;	A line will never cross a page boundary
	ld	e,0x20
ca_next:
	out	(c),e
	inc	b
	dec	a
	jr	nz, ca_next
nobytes:
	pop	bc
	ret

_clear_lines:
	ld	hl,2
	add	hl,sp
	push	bc
	ld	d,(hl)
	inc	hl
	inc	hl
	ld	a,(hl)
	or	a
	jr	z, nobytes

nextline:
	ld	e,0

	call	addr

	ld	a,0x20
	ld	e,0x40
cl_next:
	out	(c),a
	inc	b
	dec	e
	jr	nz, cl_next
	inc	d
	dec	(hl)
	jr	nz, nextline
	pop	bc
	ret

; Scrolling needs a bounce buffer as we've got no LDIR equivalent for OUT/IN

scrbuf:
	.ds	64			; would be nice to trim this

gline:
	; Lines never cross pages
	call	addr
	ld	hl,scrbuf
	; Use something else for count so we don't make a mess
	ld	a,0x10
gnext:
	ini
	ini
	ini
	ini
	dec 	a
	jr	nz, gnext
	ret

pline:
	; Lines never cross pages
	call	addr
	ld	hl,scrbuf
	; Use something else for count so we don't make a mess
	inc	b
	ld	a,0x10
pnext:
	outi
	outi
	outi
	outi
	dec 	a
	jr	nz, pnext
	ret

_scroll_down:
	push	bc
	ld	de,0x1E3F		; line 30 end
next_down:
	call	gline
	inc	d
	call	pline
	dec	d
	dec	d
	bit 	7,d
	jr	z, next_down
	pop	bc
	ret

_scroll_up:
	push	bc
	ld	de,0x013F		; second line end
next_up:
	call	gline
	dec	d
	call	pline
	inc	d
	inc	d
	ld	a,48
	cp	d
	jr	nz, next_up
	pop	bc
	ret

	.data
_vtattr_cap:
	.byte	1			; Inverse video

	.bss
cursor:
	.word	0
inverse:
	.byte	0
