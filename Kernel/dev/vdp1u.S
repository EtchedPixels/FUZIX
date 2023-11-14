	; video driver
	.export scroll_up
	.export scroll_down
	.export plot_char
	.export clear_lines
	.export clear_across
	.export cursor_on
	.export cursor_off
	.export cursor_disable

	.export cursorpos

	.export _vdpport

	.export _vdp_init
	.export _vdp_type
	.export _vdp_load_font
	.export _vdp_wipe_consoles
	.export _vdp_restore_font
	.export _vdp_text40
	.export _vdp_text32
	.export _vdp_setup40
	.export _vdp_setup32
	.export _vdp_set
	.export _vdp_readb
	.export _vdp_out
	.export _vdp_setup
	.export _vdp_set_console
	.export _vdp_setborder
	.export _vdp_setcolour

	.export _fontdata_6x8
	.export _vtattr_notify


	.code
;
;	These are with the IRQ on. Subtract 0x20 from R1 for IRQ off
;

#ifdef VDP_IRQ
_vdp_text40:
	.byte 0x00, 0xF0, 0x00, 0x00, 0x02, 0x00, 0x00, 0xF4
_vdp_text32:
	.byte 0x00, 0xE2, 0x00, 0x80, 0x02, 0x76, 0x03, 0xF4

#else
_vdp_text40:
	.byte 0x00, 0xF0, 0x00, 0x00, 0x02, 0x00, 0x00, 0xF4
_vdp_text32:
	.byte 0x00, 0xE2, 0x00, 0x80, 0x02, 0x76, 0x03, 0xF4

#endif

;
;	vdp_setup(uint8_t *table)
;
;	Initialize the VDP using the table
;
_vdp_setup:
	pop	de
	pop	hl
	push	hl	
	push	de
vdp_setup_hl:
	push	bc
	ld	d,#0x7F			; 0x80 is first register
	ld	bc, (_vdpport)
	ld	b,#16
setupl:	outi
	inc	d
	VDP_DELAY
	out	(c),d
	djnz	setupl
	pop	bc
	ret
;
;	Read a VDP byte - C
;
_vdp_readb:
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc
	call	_vdp_set
	dec	c
	in	l,(c)
	pop	bc
	ret

_vdp_out:
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc
	ld	bc,(_vdpport)
	dec	c
	out	(c),l
	pop	bc
	ret
;
;	vdp_setup40(void)
;	vdp_init(void)
;	vdp_setup32(void)
;
;	Video mode setup
;
_vdp_init:
_vdp_setup40:
	ld	hl,_vdp_text40
	call	vdp_setup_hl
	ld	hl, scrollconf40
setscroll:
	ld	de, _scrolld_base
	ld	bc, 10
	ldir
	ret

_vdp_setup32:
	ld	hl,_vdp_text32
	call _vdp_setup_hl
	ld	hl, scrollconf32
	jr	setscroll
;
;	vdp_type(void)	-	return the type of VDP present
;
_vdp_type:
	push bc
	; Program the video engine
	ld	bc,(_vdpport)
	; Play with status register 2
	ld	a,0x07
	out	(c),a
	ld	a,0x8F
	out	(c),a
	VDP_DELAY2
	in	a,(c)
	ld	a,2
	out	(c),a
	ld	a,0x8F
	out	(c),a
	VDP_DELAY2
vdpwait:
	in	a,(c)
	and	0xE0
	add	a,a
	jr	nz, not9918a	; bit 5 or 6
	jr	nc, vdpwait	; not bit 7
	; we vblanked out - TMS9918A
	ld	l,0
	pop	bc
	ret
not9918a:   ; Use the version register
	ld	a,1
	out	(c),a
	ld	a,0x8F
	out	(c),a
	VDP_DELAY2
	in	a,(c)
	rrca
	and	0x1F
	inc	a
	ld	l,a
	xor	a
	out	(c),a
	ld	a,0x8F
	out	(c),a
	pop	bc
	ret

;
;	Load the font into the stash area
;
_vdp_load_font:
	push	bc
	ld	bc,(_vdpport)
	ld	de,0x7C00
	out	(c),e			; set write and font area
	out	(c),d
	ld	hl,_fontdata_6x8
	ld	b,e
	dec	c
wipelow:			; below font
	out	(c),e
	VDP_DELAY
	djnz	wipelow
	call	lf256
	call	lf256
lf256:	; b is 0 at this point
	VDP_DELAY
	ld	a,(hl)
	inc	hl
	add	a,a
	add	a,a
	out	(c),a
	djnz	lf256
	ret

;
;	vdp_restore_font(void)
;
;	Reset the font from the cache at 3C00-3FFF in the video memory
;
_vdp_restore_font:
	push	bc
	ld	de,0x3c00
	ld	hl,0x5054
	ld	bc,(_vdpport)
	in	a,(c)
fontnext:
	out	(c),e
	out	(c),d
	VDP_DELAY2
	dec	c
	in	a,(c)
	inc	c
	out	(c),e
	out	(c),h
	dec	c
	out	(c),a
	VDP_DELAY
	inc	c
	out	(c),e
	out	(c),l
	cpl
	dec	c
	VDP_DELAY
	out	(c),a
	inc	c
	inc	e
	jr	nz, fontnext
	inc	d
	inc	h
	inc	l
	bit	6,d
	jr	z, fontnext
	ret

_vdp_wipe_consoles:
	push	bc
	ld	bc, (_vdpport)
	ld	b,0
	ld	a,0x40
	out	(c),b			; 0x0000 for writing
	out	(c),a
	dec	c
	ld	a,32
	ld	d,16			; 4K
wipe_con:
	out	(c),a
	VDP_DELAY
	djnz	wipe_con
	dec	d
	jr	nz, wipe_con
	ret

_vdp_setcolour:
	ld	bc,(_vdpport)
	ld	b,0
	ld	a,0x60			; 0x2000 in the VDP, for write
	out	(c),b
	out	(c),a
	dec	c
	ld	b,32
	ld	a,l			; Hardcoded for now
set_cmap:
	out	(c),a
	VDP_DELAY
	djnz	set_cmap
	ret
;
;	Register write value E to register A
;
_vdp_setborder:
	ld	h,0x87			; colour register
	; fall through
_vdp_set:
	ex	de, hl
vdpout:	ld	bc, (_vdpport)
	out	(c), e			; Write the data
	out	(c), d			; and then the register | 0x80
	ret

;
;	FIXME: need to IRQ protect the pairs of writes
;


videopos:	; turn E=Y D=X into HL = addr
	    ; pass B = 0x40 if writing
	    ; preserves C
	ld	a, (_outputtty)
	dec	a
	add	a,a			; 1K per screen
	add	a,a
	add	a,b
	ld	b,a
	ld	a,e			; 0-24 Y
	add	a,a
	add	a,a
	add	a,a			; x 8
	ld	l,a
	ld	h,0
	ld	a,(_vidmode)
	or	a
	jr	nz, pos32
	push	hl
	add	hl, hl			; x 16
	add	hl, hl			; x 32
	ld	a, d
	pop	de
	add	hl, de			; x 40
	ld	e, a
	ld	d, b			; 0 for read 0x40 for write
	add	hl, de			; + X
	ret
pos32:
	add	hl,hl
	add	hl,hl
	ld	e,d
	ld	d,b
	add	hl,de
	ret
;
;	Eww.. wonder if VT should provide a hint that its the 'next char'
;
#ifdef VDP_DIRECT
	.export _plot_char:
_plot_char:
#endif
plot_char:
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
	ld	a,(_int_disabled)
	push	af
	di
plotit:
	ld	b, 0x40			; writing
	call	videopos
	ld	a, c
plotit2:
	ld	bc, (_vdpport)
	out	(c), l			; address
	out	(c), h			; address | 0x40
	dec	c
	out	(c), a			; character
popret:
	pop	af
	pop	bc
	or	a
	ret	nz
	ei
	;
	;	We assume our worst case char-char delay is sufficient to
	;   skip a VDP delay here. This ought to be true even on a fast
	;	Z180 box.
	;
	ret

;
;	Painful
;
	.bss

scrollbuf:
	.ds		40

	.code
;
;	scroll_down(void)
;
#ifdef VDP_DIRECT
	.export _scroll_down:
_scroll_down:
#endif
scroll_down:
	push	bc
	ld	a,(_int_disabled)
	push	af
	di
	ld	a, (_outputtty)
	dec	a
	add	a,a			; 1K per screen
	add	a,a
	ld	h,a
	ld	l,0
	ld	b,23
	ld	de, (_scrolld_base) ; start of bottom line
	add	hl,de
	ex	de,hl
upline:
	push	bc
	ld	bc, (_vdpport)	; vdpport + 1 always holds #80
	ld	hl, scrollbuf
	out	(c), e		; our position
	out	(c), d
	dec	c
down_0:
	ini
	VDP_DELAY
	jp	nz, down_0
	inc	c
	ld	hl, (_scrolld_s1); go down one line and into write mode
	add	hl, de		; relative to our position
	out	(c), l
	out	(c), h
	; FIME - from vdpport - and into other copies
	ld	b, 40
	ld	hl, scrollbuf
	dec	c
down_1:
	outi		; video ptr is to the line below so keep going
	VDP_DELAY
	jp	nz,down_1
	pop	bc		; recover line counter
	ld	hl, (_scrolld_mov)
	add	hl, de		; up 40 bytes
	ex	de, hl		; and back into DE
	djnz	upline
	jp	popret

;
;	scroll_up(void)
;
#ifdef VDP_DIRECT
	.export _scroll_up:
_scroll_up:
#endif
scroll_up:
	push	bc
	ld	a,(_int_disabled)
	push	af
	di
	ld	a, (_outputtty)
	dec	a
	add	a		; 1K per screen
	add	a
	ld	h,a
	ld	l,0
	ld	b, 23
	ld	de, (_scrollu_w)		; start of second line (base = width)
	add	hl,de
	ex	de,hl
downline:
	push bc
	ld	bc, (_vdpport)
	ld	hl, #scrollbuf
	out (c), e
	out (c), d
	dec c
up_0:
	ini
	VDP_DELAY
	jp nz, up_0
	inc c
	ld hl, (_scrollu_mov); up w bytes in the low 12 bits, add 0x40
				; for write ( we will carry one into the top
				; nybble)
	add hl, de
	out (c), l
	out (c), h
	dec c
	ld hl, #scrollbuf
	ld a,(_scrollu_w)	; get width
	ld b, a
up_1:
	outi
	VDP_DELAY
	jp nz,up_1
	pop bc
	ld hl, (_scrollu_w)	; get width
	add hl, de
	ex de, hl
	djnz downline
	jp popret

#ifdef VDP_DIRECT
	.export _clear_lines:
_clear_lines:
#endif
clear_lines:
	ld	hl,2
	add	hl,sp
	ld	e,(hl)
	inc	hl
	inc	hl
	ld	d,(hl)
	xor a
	cp d
	ret z
	push	bc
	ld	a,(_int_disabled)
	push	af
	di
	ld	c, d
	ld	d, 0
	ld	b, 0x40
	call	videopos
	ld	e, c
	ld	bc, (_vdpport)
	out	(c), l
	out	(c), h
				; Safe on MSX 2 to loop the data with IRQ on
				; but *not* on MSX 1
	dec	c
l2:	ld	a, (_scrollu_w)
	ld	b,a
	ld	a, ' '
l1:	out	(c), a		; Inner loop clears a line, outer counts
				; need 20 clocks between writes. DJNZ is 13,
				; out is 11
	VDP_DELAY
	djnz	l1
	dec	e
	jr	nz, l2
	jp	popret

#ifdef VDP_DIRECT
	.export _clear_across:
_clear_across:
#endif
clear_across:
	ld	hl,2
	add	hl,sp
	ld	e,(hl)
	inc	hl
	inc	hl
	ld	d,(hl)
	inc	hl
	inc	hl
	xor	a
	cp	(hl)
	ret	z
	push	bc
	ld	c,(hl)
	ld	a,(_int_disabled)
	push	af
	di
	ld	b, 0x40
	call	videopos
	ld	a, c
	ld	bc, (_vdpport)
	out	(c), l
	out	(c), h
	ld	b, a
	ld	a, ' '
	dec	c
l3:	out	(c), a
	VDP_DELAY
	djnz	l3
	jp	popret
;
;	cursor_on()		-	cursor on save old char
;
#ifdef VDP_DIRECT
	.export _cursor_on:
_cursor_on:
#endif
cursor_on:
	ld	hl,2
	add	hl,sp
	ld	e,(hl)
	inc	hl
	inc	hl
	ld	d,(hl)
	ld	a,(_outputtty)
	ld	c,a
	ld	a,(_inputtty)
	cp	c
	ret	nz
	push	bc
	ld	a,(_int_disabled)
	push	af
	di
	ld	(cursorpos), de
	ld	b, 0x00			; reading
	call	videopos
	ld	a, c
	ld	bc, (_vdpport)
	out	(c), l			; address
	out	(c), h			; address
	dec	c
	VDP_DELAY2
	in	a, (c)			; character
	ld	(cursorpeek), a		; save it away
	set	6, h			; make it a write command
	xor	0x80			; write the cursor in inverse
	jp 	plotit2

;
;	cursor_off(void)	-	turn the cursor off, restore char
;
#ifdef VDP_DIRECT
	.export _cursor_off
_cursor_off:
#endif
cursor_off:
	ld	a,(_outputtty)
	ld	e,a
	ld	a,(_inputtty)
	cp	e
	ret	nz
	push	bc
	ld	a,(_int_disabled)
	push	af
	di
	ld	de, (cursorpos)
	ld	a, (cursorpeek)
	ld	c, a
	jp	plotit

#ifdef VDP_DIRECT
_vtattr_notify:
_cursor_disable:
	ret
#endif

_vdp_set_console:
	push	bc
	ld	a,(_inputtty)
	ld	bc, (_vdpport)
	dec	a
	out	(c), a
	ld	a,x82
	out	(c),a
	pop	bc
	ret

;
;	Read write low level helpers (fastcall). Need IRQ off
;
;	These are designed to run with the user space mapped so we
;	directly burst rectangles to and from the VDP. We check for
;	writes into the 3C00-3FFF area reserved for the font cache. It's
;	perhaps a silly check on an unprotected Z80 but when we get to Z280
;	it might matter rather more!
;
;	This implementation won't work with thunked memory
;
#ifdef VDP_ROP

	; graphics API
	.export _vdp_rop
	.export _vdp_wop

	.export map_proc_always
	.export map_kernel

_vdp_rop:
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc
	push	ix
	push	hl
	pop	ix
	ld	l, (ix + 2)	; VDP offset
	ld	a, (ix + 3)
	cp	0x3C
	ld	h, a
	jr	nc, bounds	; Offset exceeds boundary
	ld	e, (ix + 6)		; Stride
	ld	d, 0
	ld	bc, (_vdpport)
	out	(c), l
	out	(c), h		; Set starting pointer
	exx
	ld	l, (ix + 0)		; User pointer
	ld	h, (ix + 1)
	ld	d, (ix + 5)		; cols
	ld	e, (ix + 4)		; lines
	ld	bc,(_vdpport);
	inc	c		; data port
	call	map_proc_always
ropl:
	ld	b,d
ropc:
	ini
	VDP_DELAY
	djnz	ropc
	exx
	add	hl, de		; stride
	ld	a, 0x3B		; we don't care about a read overruning 3C
	cp	h		; just 3F
	jr	c, bounds
	out	(c), l		; next line
	out	(c), h
	exx
	dec	e
	jr	nz, ropl
	pop	ix
	pop	bc
	ld	hl, 0
	jp	map_kernel
bounds:
	pop	ix
	pop	bc
	ld	hl, -1
	jp	map_kernel

_vdp_wop:
	pop	de
	pop	hl
	push	hl	
	push	de
	push	bc
	push	ix
	push	hl
	pop	ix
	ld	l, (ix +2)		; VDP offset
	ld	a, 3ix + 3)
	or	0x40		; Write operation
	cp	0x7C
	ld	h, a
	jr	nc, bounds	; Offset exceeds boundary
	ld	bc, (_vdpport)
	ld	b, (ix + 5)		; cols
	ld	e, (ix + 6)		; Stride
	ld	d, 0
	out	(c),l
	out	(c),h		; Set starting pointer
	exx
	ld	l,(ix + 0)		; User pointer
	ld	h,(ix + 1)
	ld	d,(ix + 5)		; cols
	ld	e,(ix + 4)		; lines
	ld	bc,(_vdpport)	;
	inc	c		; data port
	call	map_proc_always
wopl:
	ld	b,d
wopc:
	outi
	VDP_DELAY
	djnz	wopc
	exx
	add	hl,de		; stride
	ld	a,0x7C
	cp	h
	jr	nc, boundclear
	jr	c, bounds	; going over boundary
	; We could cross the boundary during the line
	ld	a, l
	add	a,b
	jr	c, bounds
boundclear:
	out	(c), l		; next line
	out	(c), h
	exx
	dec	e
	jr	nz, ropl
	pop	ix
	pop	bc
	ld	hl,0
	jp map_kernel

#endif

;
;	This must be in data or common not code
;	Note: do not put them in common space on a thunked platform or any
;	other where the common data is per instance
;
	.data
cursorpos:
	.word	0
cursorpeek:
	.byte	0

_scrolld_base:
	.dw	0x03C0		; Start of bottom line
_scrolld_mov:
	.dw	0xFFD8		; -40
_scrolld_s1:
	.dw	0x4028		; 4000 turns on write 40 chars
_scrollu_w:
	.dw	0x0028		; width
_scrollu_mov:
	.dw	0x3FD8		; up 40 bytes in low 12, add 0x4000
					; carry 11->12

scrollconf40:
	.dw 0x03C0, 0xFFD8, 0x4028, 0x0028, 0x3FD8
scrollconf32:
	.dw 0x02E0, 0xFFE0, 0x4020, 0x0020, 0x3FE0
