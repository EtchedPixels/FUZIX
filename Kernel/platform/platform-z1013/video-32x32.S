#include "kernelu.def"
#ifndef CONFIG_VIDEO_POPPE

	.common

vpos:
	ld	h,0
	ld	l,e
	add	hl,hl		; 32 * Y
	add	hl,hl
	add	hl,hl
	add	hl,hl
	add	hl,hl
	ld	e,d
	ld	d, 0
	add	hl,de		; + X
	ld	de, 0xEC00
	add	hl,de
video_on:
	di			; Protect port 4 control
	in	a,(4)
	and	0x7F
	out	(4),a
	ret

	.export _video_init

_video_init:
	ret

	.export	_plot_char

_plot_char:
	ld	hl,2
	add	hl,sp
	push	bc
	ld	e,(hl)
	inc	hl
	inc	hl
	ld	d,(hl)
	inc	hl
	inc	hl
	ld	c,(hl)
	call	vpos
	ld	(hl),c
vidout:
	pop	bc
vidout2:
	in	a,(4)
	or	0x80
	out	(4),a
	ld	a,(_int_disabled)
	or	a
	ret	nz
	ei
	; fall through

	.export	_vtattr_notify

_vtattr_notify:
	ret

	.export	_clear_lines

_clear_lines:
	ld	hl,2
	add	hl,de
	push	bc
	ld	e,(hl)
	inc	hl
	inc	hl
	ld	c,(hl)
	inc	c
	ld	d,0		; X = 0 for finding line start
	call	vpos
	ld	a,0x20		; space
	ld	de,0x20	; line width
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

	.export	_clear_across

_clear_across:
	ld	hl,2
	add	hl,de
	push	bc
	ld	e,(hl)
	inc	hl
	inc	hl
	ld	d,(hl)
	inc	hl
	inc	hl
	ld	c,(hl)
	call	vpos
	ld	b,c
	inc	b
	ld	a,0x20		; space
	jr	nchar
ca:	ld	(hl),a
	inc	hl
nchar:	djnz	ca
	jr	vidout

	.export	_scroll_down

_scroll_down:
	call	video_on
	ld	hl,0xEFFF	; last char
	ld	de,0xEFDF	; line above last char
	ld	bc,0x3E0	; size to copy
	lddr
	jr	vidout2

	.export _scroll_up

_scroll_up:
	call	video_on
	ld	hl,0xEC20	; line 1
	ld	de,0xEC00	; top line
	ld	bc,0x03E0
	ldir
	jr	vidout2

	.export _cursor_on
_cursor_on:
	ld	hl,2
	add	hl,sp
	ld	e,(hl)
	inc	hl
	inc	hl
	ld	d,(hl)
	call	vpos
	ld	(cursorpos),hl
	ld	a,(hl)
	ld	(cursorchar),a
	ld	(hl),0		; solid block
	jr	vidout

	.export	_cursor_disable
	.export	_cursor_off

_cursor_disable:
_cursor_off:
	call	video_on
	ld	hl,(cursorpos)
	ld	a,(cursorchar)
	ld	(hl),a
	jr	vidout

cursorpos:
	.word	0
cursorchar:
	.byte	0

#endif
