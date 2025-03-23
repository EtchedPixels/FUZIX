;
;	Use the 80 column text mode only for now
;

	.export _cursor_off
	.export _cursor_disable
	.export _cursor_on
	.export _plot_char
	.export _clear_lines
	.export _clear_across
	.export _vtattr_notify
	.export _scroll_up
	.export _scroll_down

#include "../../build/kernelu.def"

	.common
;
;	Required to preserve BC
;
addr:
	ld	a,e		; get Y
	add	a,a
	add	a,a
	add	a,a
	add	a,a		; x16: as far as we can 8bit add
	ld	l,a
	ld	h,0
	push	hl
	ld	e,d
	ld	d,h		; DE is now 00xx where xx is the X value
	add	hl,hl	; x 32
	add	hl,hl	; x 64
	add	hl,de	; x 64 + X
	pop	de
	add	hl,de	; x 80 + X
	set	6,h	; + 0x4000
	di
	ld	a,21
	out	(0xF1),a	; 0x4000 is now video RAM
	ret

_cursor_off:
	ld	hl,(cpos)
	bit	6,h		; all valid cpos values are > 0x4000
	ret	z
	ld	a,(csave)
	ld	(hl),a
	xor	a
	ld	(cpos+1),a
vidout:
	; Restore the memory mappings
	ld	a,(mpgsel_cache + 1)
	out	(0xF1),a
	ld	a,(_int_disabled)
	or	a
	ret	nz
	ei
_vtattr_notify:
_cursor_disable:
	ret

;
;	TOS is Y X
;
_cursor_on:
	ld	hl,#4
	add	hl,sp
	ld	d,(hl)
	dec	hl
	dec	hl
	ld	e,(hl)
	call	addr
	ld	a,(hl)
	ld	(hl),'_'
	ld	(csave),a
	ld	(cpos),hl
	jr	vidout

;
;	TOS is Y X C
;
_plot_char:
	ld	hl,#6
	add	hl,sp
	ld	a,(hl)
	dec	hl
	dec	hl
	ld	d,(hl)
	dec	hl
	dec	hl
	ld	e,(hl)
	; Y in E X in D char in A
	push	af
	call	addr
	pop	af
	ld	(hl),a
	jr	vidout

;
;	TOS is Y count
;
_clear_lines:
	; TODO: do this in one ldir
	push	bc	;	save register variable
	ld	hl,#6
	add	hl,sp
	ld	c,(hl)	;	count
	dec	hl
	dec	hl
	ld	e,(hl)	;	Y
	ld	d,0	;	X
	call	addr
	ld	a,c	; A is now line count
	or	a
	jr	z, noclear
wipeline:
	ld	b,80
wiper:	ld	(hl),' '
	inc	hl
	djnz	wiper
	dec	a
	jr	nz, wipeline
noclear:
	pop	bc
	jr	vidout

;	TOS is Y X count
_clear_across:
	push	bc
	ld	hl,#8
	add	hl,sp
	ld	b,(hl)
	dec	hl
	dec	hl
	ld	d,(hl)
	dec	hl
	dec	hl
	ld	e,(hl)
	; Y in E X in D count in B
	ld	a,b
	or	a
	jr	z, noclear
	call	addr
	ld	a,' '
clear2:	ld	(hl),a
	inc	hl
	djnz	clear2
	pop	bc
	jr	vidout

_scroll_up:
	push	bc
	di
	ld	a,21
	out	(0xF1),a	; 0x4000 is now video RAM
	ld	hl,0x4050
	ld	de,0x4000
	ld	bc,0x0730
	ldir
	pop	bc
	jr	vidout

_scroll_down:
	push	bc
	di
	ld	a,21
	out	(0xF1),a	; 0x4000 is now video RAM
	ld	hl,0x477F
	ld	de,0x472F
	ld	bc,0x0730
	lddr
	pop	bc
	jr	vidout

	.commondata		; So it's not at risk of vanishing
				; during a video op

csave:
	.byte	0
cpos:
	.word	0
