
	.module _video

	.globl	map_process_always
	.globl	map_kernel

;
;	Video RAM is E800-EFFF with a separate colour bank we do not yet
;	bother with except to set a default. The card memory is not read
;	but is shadowed by E800-EFFF in the main system when reading. This
;	means that
;	1. We need to be careful what is mapped there
;	2. It is not possible to do attributes nicely because we would have to
;	   store an extra copy of the memory somewhere
;
;	FIXME: do we need to force 0x04 bit 7 in the vpos etc code and juggle
;	the irq state ?
;

	.area _DISCARD
	.globl _video_init

_video_init:
	call	map_process_always
	ld	a,#1
	out	(0x12),a
	ld	hl,#0xE800
	ld	de,#0xE801
	ld	bc,#0x800
	ld	(hl),#0x02
	ldir
	xor	a
	out	(0x12),a
	ld	hl,#0xE800
	ld	de,#0xE801
	ld	bc,#0x0800
	ld	(hl),#0x20
	ldir
	out	(0x12),a
	out	(0x11),a
	jp	map_kernel

	.area	_COMMONMEM

vpos:
	ld	h,#0
	ld	l,e
	add	hl,hl		; 64 * Y
	add	hl,hl
	add	hl,hl
	add	hl,hl
	add	hl,hl
	add	hl,hl
	ld	e,d
	ld	d,#0
	add	hl,de		; + X
	ld	de,#0xE800
	add	hl,de
	jp	map_process_always


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
	jp	map_kernel

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
;
;	Clear entire text lines in the current bank. Called twice, once
;	to wipe the display and once to set the colour memory
;
	ld	de,#0x40	; line width
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
	jp	map_kernel

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
	jp	map_kernel

	.globl	_scroll_down

_scroll_down:
	call	map_process_always
	ld	hl,#0xEFFF	; last char
	ld	de,#0xEF7F	; line above last char
	ld	bc,#0x7C0	; size to copy
	lddr
	jp	map_kernel

	.globl _scroll_up

_scroll_up:
	call	map_process_always
	ld	hl,#0xE840	; line 1
	ld	de,#0xE800	; top line
	ld	bc,#0x07C0
	ldir
	jp	map_kernel

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
	ld	(hl),#'_'
	jp	map_kernel

	.globl	_cursor_disable
	.globl	_cursor_off

_cursor_disable:
_cursor_off:
	call	map_process_always
	ld	hl,(cursorpos)
	ld	a,(cursorchar)
	ld	(hl),a
	jp	map_kernel

cursorpos:
	.dw	0
cursorchar:
	.db	0
