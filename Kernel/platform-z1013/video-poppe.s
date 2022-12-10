
	.module _video

	.globl	map_process_always
	.globl	map_kernel

;
;	Video RAM is E800-EFFF with a separate colour bank we don't yet
;	bother with except to set a default
;
;	Only visible when the ROM (ie kernel) is unmapped
;
;	Curiously this means we can actually do dual monitor in theory
;	because EC00-EFFF is
;
;	32K ROM		OS ROM
;	IN		IN		Main video
;	IN		OUT		32K ROM
;	OUT		IN		Both
;	OUT		OUT		Poppe video
;

	.area _COMMONMEM

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

	.globl _video_init

_video_init:
	call	map_process_always
	xor	a
	out	(0x12),a
	ld	hl,#0xE800
	ld	de,#0xE801
	ld	bc,#0x800
	ld	(hl),#0x20
	ldir
	inc	a
	out	(0x12),a
	ld	hl,#0xE800
	ld	de,#0xE801
	ld	bc,#0x0800
	ld	(hl),#0x20
	ldir
	dec	a
	out	(0x12),a
;	ld	a,#0x02
	out	(0x11),a
	jp	map_kernel

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
	ld	de,#0xEFDF	; line above last char
	ld	bc,#0x7E0	; size to copy
	lddr
	jp	map_kernel

	.globl _scroll_up

_scroll_up:
	call	map_process_always
	ld	hl,#0xE820	; line 1
	ld	de,#0xE800	; top line
	ld	bc,#0x07E0
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
	ld	hl,(cursorpos)
	ld	a,(cursorchar)
	ld	(hl),a
	jp	map_kernel

cursorpos:
	.dw	0
cursorchar:
	.db	0
