;
;	Do this in assembler as we bank video in and out.
;
	.module vtsupport

	.globl _vtswap
	.globl vtbufinit
	.globl _vtbase
	.globl _curtty

	.globl _cursor_off
	.globl _cursor_disable
	.globl _cursor_on
	.globl _plot_char
	.globl _clear_lines
	.globl _clear_across
	.globl _vtattr_notify
	.globl _scroll_up
	.globl _scroll_down

	.globl map_mmio
	.globl unmap_mmio

	.area _COMMONMEM
	; So we can keep it visible with MMIO mapped
_vtbackbuf:
	.ds	1024

	.area _COMMONMEM
_vtbase:
	.dw	0x3C00
	.dw	_vtbackbuf

	.area _VIDEO
_vtswap:
	call	map_mmio
	ld	hl, #0x3C00
	ld	de, #_vtbackbuf
	ld	bc, #1024	; 64 * 16
exchit:
	push	bc
	ld	a, (de)	; Could be optimised but its only 1K
	ld	c, (hl)	; Probably worth doing eventuallly
	ex	de, hl
	ld	(hl), c
	ld	(de), a
	inc	hl
	inc	de
	pop	bc
	dec	bc
	ld	a, b
	or	c
	jr	nz, exchit
	jp	unmap_mmio

vtbufinit:
	ld	hl,#_vtbackbuf
	ld	de,#_vtbackbuf+1
	ld	bc,#1023
	ld	(hl),#' '
	ldir
	ret
;
;	This is basically VT_SIMPLE but done in asm so we know all our
;	I/O happens with the right things mapped
;

vtbase:	
	ld	hl,(_vtbase)
	ld	a,(_curtty)
	or	a
	ret	z
	ld	hl,(_vtbase + 2)
_cursor_disable:
_vtattr_notify:
	ret
;
;	Must preserve BC
;	Turn D,E into a display address
;
addr:	
	call	vtbase
	ld	a,d	; save X
	ld	d,e
	ld	e,#0
	srl	d	;  DE = Y * 64
	rr	e
	srl	d
	rr	e
	add	e	; add in X
	ld	e,a
	add	hl,de	; add in base
	jp	map_mmio

_cursor_off:
	ld	hl,(cpos)
	ld	a,h
	or	a	;	00xx isn't valid so no need to check l
	ret	z
	di
	call	map_mmio
	ld	a,(csave)
	ld	(hl),a
	xor	a
	ld	(cpos+1),a
	jp	unmap_mmio

_cursor_on:
	pop	bc
	pop	de
	push	de
	push	bc
	call	addr
	ld	a,(hl)
	ld	(hl),#'_'
	ld	(csave),a
	ld	(cpos),hl
	jp	unmap_mmio

_plot_char:
	pop	hl
	pop	de
	pop	bc
	push	bc
	push	de
	push	hl
	call	addr
	ld	(hl),c
	j	unmap_mmio

_clear_lines:
	pop	bc
	pop	de		; E = y D = count
	push	de
	push	bc
	ld	c,d
	ld	d,#0
	call	addr
	ld	a,#32
clear0:
	ld	b,#64		; line width
clear1:
	ld	(hl),a
	inc	hl
	djnz	clear1
	dec	c
	jr	nz, clear0
	jp	unmap_mmio

_clear_across:
	pop	hl
	pop	de		; E = y, D = x
	pop	bc		; C = count
	push	bc
	push	de
	push	hl
	call	addr
	ld	a,#32
	ld	b,c
clear2:	ld	(hl),a
	inc	hl
	djnz	clear2
	jr	unmap_video

_scroll_up:
	call	vtbase		; HL now the base of the video
	call	map_mmio
	push	hl
	ld	de,#64
	add	hl,de		; HL is now the video second line
	pop	de		; top of video (destination)
	ld	bc,#1024-64
	ldir
	jp	unmap_mmio

_scroll_down:
	call	vtbase
	call	map_mmio
	ld	de,#1024-64		
	add	hl,de		; points to start of last line
	dec	hl		; end of line before last
	push	hl
	ld	de,#64
	add	hl,de		; last char
	pop	de
	ex	de,hl
	ld	bc,#1024-64
	lddr
	jp	unmap_mmio

csave:	.byte	0
cpos:	.word	0

