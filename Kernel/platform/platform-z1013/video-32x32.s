
	.module _video

	.area _COMMONMEM

	.globl	_int_disabled

vpos:
	ld	h,#0
	ld	l,e
	add	hl,hl		; 32 * Y
	add	hl,hl
	add	hl,hl
	add	hl,hl
	add	hl,hl
	ld	e,d
	ld	d,#0
	add	hl,de		; + X
	ld	de,#0xEC00
	add	hl,de
video_on:
	di			; Protect port 4 control
	in	a,(4)
	and	a,#0x7F
	out	(4),a
	ret

	.globl _video_init

_video_init:
	ret

	.globl	_plot_char

_plot_char:
	pop	hl
	pop	de
	pop	bc		; c = char
	push	bc
	push	de		; D = X E = Y
	push	hl
	call	vpos
	ld	(hl),c
vidout:
	in	a,(4)
	or	a,#0x80
	out	(4),a
	ld	a,(_int_disabled)
	or	a
	ret	nz
	ei
	; fall through

	.globl	_vtattr_notify

_vtattr_notify:
	ret

	.globl	_clear_lines

_clear_lines:
	pop	hl
	pop	de		; E = line, D = count
	push	de
	push	hl
	ld	c,d
	inc	c
	ld	d,#0		; X = 0 for finding line start
	call	vpos
	ld	a,#0x20		; space
	ld	de,#0x20	; line width
	jr	nextline
cl:	push	hl
	ld	b,e		; line width
c2:	ld	(hl),a
	inc	hl
	djnz	c2
	pop	hl
	add	hl,de
nextline:
	dec	c
	jr	nz,cl
	jr	vidout

	.globl	_clear_across

_clear_across:
	pop	hl
	pop	de		; D,E = x,y
	pop	bc		; C = count
	push	bc
	push	de
	push	hl
	call	vpos
	ld	b,c
	inc	b
	ld	a,#0x20		; space
	jr	nchar
ca:	ld	(hl),a
	inc	hl
nchar:	djnz	ca
	jr	vidout

	.globl	_scroll_down

_scroll_down:
	call	video_on
	ld	hl,#0xEFFF	; last char
	ld	de,#0xEFDF	; line above last char
	ld	bc,#0x3E0	; size to copy
	lddr
	jr	vidout

	.globl _scroll_up

_scroll_up:
	call	video_on
	ld	hl,#0xEC20	; line 1
	ld	de,#0xEC00	; top line
	ld	bc,#0x03E0
	ldir
	jr	vidout

	.globl _cursor_on
_cursor_on:
	pop	hl
	pop	de
	push	de
	push	hl
	call	vpos
	ld	(cursorpos),hl
	ld	a,(hl)
	ld	(cursorchar),a
	ld	(hl),#0		; solid block
	jr	vidout

	.globl	_cursor_disable
	.globl	_cursor_off

_cursor_disable:
_cursor_off:
	call	video_on
	ld	hl,(cursorpos)
	ld	a,(cursorchar)
	ld	(hl),a
	jr	vidout

cursorpos:
	.dw	0
cursorchar:
	.db	0
