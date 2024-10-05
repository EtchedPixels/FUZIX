	; video driver
	.export	scroll_up
	.export	scroll_down
	.export	plot_char
	.export	clear_lines
	.export	clear_across
	.export	cursor_on
	.export	cursor_off

	; graphics API
	.export	_vdp_rop
	.export	_vdp_wop

	.export	cursorpos

	.export _vdp_init
	.export _vdp_type
	.export	_vdp_load_font
	.export	_vdp_wipe_consoles
	.export	_vdp_restore_font
	.export	_vdp_text40
	.export	_vdp_text32
	.export	_vdp_setup40
	.export	_vdp_setup32
	.export	_vdp_set
	.export	_vdp_readb
	.export	_vdp_out
	.export	_vdp_setup
	.export	_vdp_set_console
	.export	_vdp_setborder
	.export	_vdp_setcolour

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
	pop	d
	pop	h
	push	h
	push	d
vdp_setup:
	mvi	d,0x80			; 0x80 is first register
	mvi	b,8
setupl:	mov	a,m
	out	VDP_CTRL
	mov	a,d
	out	VDP_CTRL
	inr	d
	inx	h
	dcr	b
	jnz	setupl
	ret
;
;	Read a VDP byte - C
;
_vdp_readb:
	call	_vdp_set
	in	VDP_DATA
	mov	l,a
	ret

_vdp_out:
	pop	h
	pop	d
	push	d
	mov	a,e
	out	VDP_DATA
	pchl

;
;	vdp_setup40(void)
;	vdp_init(void)
;	vdp_setup32(void)
;
;	Video mode setup
;
_vdp_init:
_vdp_setup40:
	push	b
	lxi	h,_vdp_text40
	call	vdp_setup
	lxi	h,scrollconf40
setscroll:
	lxi	d,_scrolld_base
	mvi	b,10
setlp:
	mov	a,m
	stax	d
	inx	h
	inx	d
	dcr	b
	jnz	setlp
	pop	b
	ret

_vdp_setup32:
	push	b
	lxi	h,_vdp_text32
	call	vdp_setup
	lxi	h,scrollconf32
	jmp	setscroll
;
;	vdp_type(void)	-	return the type of VDP present
;
_vdp_type:
	; Play with status register 2
	mvi	a,0x07
	out	VDP_CTRL
	mvi	a,0x8F
	out 	VDP_CTRL
	VDP_DELAY2
	in	VDP_CTRL
	mvi	a,2
	out	VDP_CTRL
	mvi	a,0x8F
	out	VDP_CTRL
	VDP_DELAY2
vdpwait:
	in	VDP_CTRL
	ani	 0xE0
	add	a
	jnz	not9918a	; bit 5 or 6
	jnc	vdpwait		; not bit 7
	; we vblanked out - TMS9918A
	mvi	l,0
	ret
not9918a:   ; Use the version register
	mvi	a,1
	out	VDP_CTRL
	mvi	a,0x8F
	out	VDP_CTRL
	VDP_DELAY2
	in	VDP_CTRL
	rrc
	ani	0x1F
	inr	a
	mov	l,a
	xra	a
	out	VDP_CTRL
	mvi	a,0x8F
	out	VDP_CTRL
	ret

;
;	Load the font into the stash area
;
_vdp_load_font:
	push	b
	lxi	d,0x7C00
	mov	a,e
	out	VDP_CTRL	; Set write and font area
	mov	a,d
	out	VDP_CTRL
	lxi	h,_fontdata_6x8
	mov	b,e		; B = 0
wipelow:			; below font
	mov	a,e
	out	VDP_DATA
	VDP_DELAY
	dcr	b
	jnz	wipelow
	call	lf256
	call	lf256
	call	lf256
	pop	b
	ret
lf256:	; b is 0 at this point
	VDP_DELAY
        mov	a,m
	inx	h
	add	a
	add	a
	out	VDP_DATA
	dcr	b
	jnz	lf256
	ret
;
;	vdp_restore_font(void)
;
;	Reset the font from the cache at 3C00-3FFF in the video memory
;
_vdp_restore_font:
	push	b
	lxi	d,0x3c00
	lxi	h,0x5054
	in	VDP_CTRL
fontnext:
	mov	a,e
	out	VDP_CTRL
	mov	a,d
	out	VDP_CTRL	
	VDP_DELAY2
	in	VDP_DATA
	mov	c,a
	mov	a,e
	out	VDP_CTRL
	mov	a,h
	out	VDP_CTRL
	mov	a,c
	out	VDP_DATA
	VDP_DELAY
	mov	a,e
	out	VDP_CTRL
	mov	a,l
	out	VDP_CTRL
	mov	a,c
	cma
	out	VDP_DATA
	VDP_DELAY
	inr	c
	inr	e
	jnz	fontnext
	inr	d
	inr	h
	inr	l
	mov	a,d
	ani	0x40
	jz	fontnext
	pop	b
	ret

_vdp_wipe_consoles:
	push	b
	xra	a
	out	VDP_CTRL
	mov	b,a
	mvi	a,0x40
	out	VDP_CTRL
	mvi	a,32
	mvi	d,16			; 4K
wipe_con:
	out	VDP_DATA
	VDP_DELAY
	dcr	b
	jnz	wipe_con
	dcr	d
	jnz	wipe_con
	pop	b
	ret

_vdp_setcolour:
	pop	h
	pop	d
	push	d
	push	b
	xra	a
	out	VDP_CTRL
	mov	b,a
	mvi	a,0x60			; 0x2000 in the VDP, for write
	out	VDP_CTRL
	mvi	b,32
	mov	a,e			; Hardcoded for now
set_cmap:
	out	VDP_DATA
	VDP_DELAY
	dcr	b
	jnz	set_cmap
	pop	b
	pchl

_vdp_setborder:
	xra	a
	out	VDP_CTRL
	mvi	a,0x87			; colour register
	out	VDP_CTRL
	ret

_vdp_set:
	pop	h
	pop	d
	push	d
vdpout:
	mov	a,e
	out	VDP_CTRL
	mov	a,d
	out	VDP_CTRL
	pchl

;
;	FIXME: need to IRQ protect the pairs of writes
;


videopos:	; turn E=Y D=X into HL = addr
	        ; pass B = 0x40 if writing
	        ; preserves C
	lda	_outputtty
	dcr	a
	add	a			; 1K per screen
	add	a
	add	b
	mov	b,a
	mov	a,e			; 0-24 Y
	add	a
	add	a
	add	a			; x 8
	mov	l, a
	mvi	h, 0
	lda	_vidmode
	ora	a
	jnz	pos32
	push	h
	dad	h			; x 16
	dad	h			; x 32
	mov	a, d
	pop	d
	dad	d			; x 40
	mov	e, a
	mov	d, b			; 0 for read 0x40 for write
	dad	d			; + X
	ret
pos32:
	dad	h
	dad	h
	mov	e,d
	mov	d,b
	dad	d
	ret
;
;	Eww.. wonder if VT should provide a hint that its the 'next char'
;
#ifdef	VDP_DIRECT
	.export _plot_char
_plot_char:
#endif
plot_char:
	push	b
	lxi	h,4
	dad	sp
	mov	e,m
	inx	h
	inx	h
	mov	d,m
	inx	h
	inx	h
	mov	c,m
	lda	_int_disabled
	push	psw
	di
plotit:
	mvi	b, 0x40			; writing
	call	videopos
plotit2:
	mov	a,l
	out	VDP_CTRL		; address
	mov	a,h
	out	VDP_CTRL		; address | 0x40
	mov	a,c
	out	VDP_DATA		; character
popbret:
	pop	psw
	pop	b
	ora	a
	rnz
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
	.ds	40

	.code
;
;	scroll_down(void)
;
#ifdef	VDP_DIRECT
	.export	_scroll_down
_scroll_down:
#endif
scroll_down:
	push	b
	lda	_outputtty
	dcr	a
	add	a			; 1K per screen
	add	a
	mov	h,a
	mvi	l,0
	mvi	b,23
	lxi	d,_scrolld_base 	; start of bottom line
	dad	d
	xchg
upline:
	push	b
	lda	_scrollu_w		; Width
	mov	b,a
	lxi	h, scrollbuf
	mov	a,e
	out	VDP_CTRL		; our position
	mov	a,d
	out	VDP_CTRL
down_0:
	in	VDP_DATA
	mov	m,a
	inx	h
	VDP_DELAY
	dcr	b
	jnz	down_0
	lhld	_scrolld_s1	; go down one line and into write mode
	dad	d		; relative to our position
	mov	a,l
	out	VDP_CTRL
	mov	a,h
	out	VDP_CTRL
	lda	_scrollu_w	; Width
	lxi	h, scrollbuf
down_1:
	mov	a,m
	out	VDP_DATA	; video ptr is to the line below so keep going
	inx	h
	VDP_DELAY
	dcr	b
	jnz	down_1
	pop	b		; recover line counter
	lhld	_scrolld_mov
	dad	d		; up 40 bytes
	xchg			; and back into DE
	dcr	b
	jnz	upline
	jmp	popbret

;
;	scroll_up(void)
;
#ifdef VDP_DIRECT
	.export	_scroll_up
_scroll_up:
#endif
scroll_up:
	push	b
	lda	_int_disabled
	push	psw
	di
	lda	_outputtty
	dcr	a
	add	a			; 1K per screen
	add	a
	mov	h,a
	mvi	l,0
	mvi	b,23
	xchg
	lhld	_scrollu_w		; start of second line (base = width)
	dad	d
	xchg
downline:
	push	b
	lda	_scrollu_w
	mov	b,a
	lxi	h, scrollbuf
	mov	a,e
	out	VDP_CTRL
	mov	a,d
	out	VDP_CTRL
up_0:
	in	VDP_DATA
	mov	m,a
	inx	h
	VDP_DELAY
	dcr	b
	jnz	up_0

	lhld	_scrollu_mov	; up w bytes in the low 12 bits, add 0x40
				; for write ( we will carry one into the top
				; nybble)
	dad	d
	mov	a,l
	out	VDP_CTRL
	mov	a,h
	out	VDP_CTRL
	lxi	h, scrollbuf
	lda 	_scrollu_w	; get width
	mov	b, a
up_1:
	mov	a,m
	out	VDP_DATA
	inx	h
	VDP_DELAY
	dcr	b
	jnz	up_1
	pop	b
	lhld	_scrollu_w	; get width
	dad	d
	xchg
	dcr	b
	jnz	downline
	jmp	popbret

#ifdef VDP_DIRECT
	.export	_clear_lines
_clear_lines:
#endif
clear_lines:
	push	b
	lxi	h,4
	dad	sp
	mov	e,m
	inx	h
	inx	h
	mov	c,m
	xra	a
	cmp	c
	jz	nowork

	lda	_int_disabled
	push	psw
	di
	mvi	d, 0
	mvi	b, 0x40
	call	videopos
	mov	e, c
	mov	a,l
	out	VDP_CTRL
	mov	a,h
	out	VDP_CTRL
	; Safe on MSX 2 to loop the data with IRQ on
	; but *not* on MSX 1
l2:	lda	_scrollu_w
	mov	b,a
	mvi	a, ' '
l1:	out	VDP_DATA	; Inner loop clears a line, outer counts
				; need 20 clocks between writes
	VDP_DELAY
	dcr	b
        jnz	l1
	dcr	e
	jnz	l2
	jmp	popbret
nowork:
	pop	b
	ret

#ifdef	VDP_DIRECT
	.export	_clear_across
_clear_across:
#endif
clear_across:
	push	b
	lxi	h,4
	dad	sp
	mov	e,m
	inx	h
	inx	h
	mov	d,m
	inx	h
	inx	h
	mov	c,m

	;	No lines ?
	xra	a
	cmp	c
	jz	nowork

	lda	_int_disabled
	push	psw
	di
	mvi	b, 0x40
	call	videopos
	lda	_scrollu_w		; Width
	mov	b,a
	mov	a,l
	out	VDP_CTRL
	mov	a,h
	out	VDP_CTRL
	mvi	a, ' '
l3:	out	VDP_DATA
	VDP_DELAY
	dcr	b
	jnz	l3
	jmp	popbret

;
;	cursor_on()		-	cursor on save old char
;
#ifdef	VDP_DIRECT
	.export	_cursor_on
_cursor_on:
#endif
cursor_on:
	push	b
	lxi	h,4
	dad	sp
	mov	e,m
	inx	h
	inx	h
	mov	d,m
	lda	_outputtty
	mov	c,a
	lda	_inputtty
	cmp	c
	jnz	nowork
	lda	_int_disabled
	push	psw
	di
	xchg
	shld	cursorpos
	xchg
	mvi	b, 0x00			; reading
	call	videopos
	mov	a,l
	out	VDP_CTRL		; address
	mov	a,h
	out	VDP_CTRL		; address
	VDP_DELAY2
	in	VDP_DATA		; character
	sta	cursorpeek		; save it away
	xri	0x80			; write the cursor in inverse
	mov	c,a
	mvi	a,0x40
	add	h
	mov	h,a			; make it a write command
	mov	a,c
	jmp	plotit2

;
;	cursor_off(void)	-	turn the cursor off, restore char
;
#ifdef VDP_DIRECT
	.export	_cursor_off
_cursor_off:
#endif
cursor_off:
	push	b
	lda	_outputtty
	mov	c,a
	lda	_inputtty
	cmp	c
	rnz
	lda	_int_disabled
	push	psw
	di
	xchg
	lhld	cursorpos
	xchg
	lda	cursorpeek
	mov	c, a
	jmp	plotit

#ifdef	VDP_DIRECT
_vtattr_notify:
_cursor_disable:
	    ret
#endif

_vdp_set_console:
	lda	_inputtty
	dcr	a
	out	VDP_CTRL
	mvi	a,0x82
	out	VDP_CTRL
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

opsetup:
	mov	e,m		; 0(x)
	inx	h
	mov	d,m		; 1(x) - user pointer
	push	d
	inx	h
	mov	e,m		; 2(x)
	inx	h
	mov	a,m		; 3(x) - start
	cpi	0x3C		; range check
	mov	d,a
	jnc	bounds
	xchg
	shld	vdpoff
	xchg
	inx	h
	mov	e,m		; lines
	inx	h
	mov	d,m		; cols
	inx	h
	mov	c,m
	pop	h
	ret

	; DE is lines cols
	; HL is userptr
	; C is stride
	; and the variables are set for offset


	.common

_vdp_rop:
	pop	d
	pop	h
	push	h
	push	d
	push	b		; save register variables
	call	opsetup
	
	push	h
	lhld	vdpoff
	mov	a,l
	out	VDP_CTRL
	mov	a,h
	out	VDP_CTRL

	call	map_proc_always

ropl:
	xthl			; vdpoff to top, vdpuptr to HL
	mov	b,d
ropc:
	in	VDP_DATA
	mov	m,a
	inx	h
	VDP_DELAY
	dcr	b
	jnz	ropc

	xthl			; vdpuptr to top, vdpoff to HL
	dad	b		; B is 0 at this point
	mvi	a, 0x3B		; we don't care about a read overruning 3C
	cmp	h		; just 3F
	jc	bounds
	mov	a,l
	out	VDP_CTRL
	mov	a,h
	out	VDP_CTRL
	dcr	e
	jnz	ropl		; Next line
	pop	h
	lxi	h,0
	pop	b
	jmp	map_kernel
bounds:
	pop	h
	lxi	h,-1
	pop	b
	jmp	map_kernel

_vdp_wop:
	pop	d
	pop	h
	push	h
	push	d
	push	b		; save register variables
	call	opsetup
	
	push	h
	lhld	vdpoff
	mov	a,l
	out	VDP_CTRL
	mov	a,h
	out	VDP_CTRL

	call	map_proc_always

wopl:
	xthl			; vdpoff to top, vdpuptr to HL
	mov	b,d
wopc:
	mov	a,m
	out	VDP_DATA
	inx	h
	VDP_DELAY
	dcr	b
	jnz	wopc

	xthl			; vdpuptr to top, vdpoff to HL
	dad	b		; B is 0 at this point
	mov	a, h		; we don't care about a read overruning 3C
	cpi	0x3C		; just 3F
	jnc	bounds
	add	d
	cpi	0x3C		; Check for overrun on this line
	jnc	bounds
	mov	a,l
	out	VDP_CTRL
	mov	a,h
	out	VDP_CTRL
	dcr	e
	jnz	wopl		; Next line
	pop	h
	lxi	h,0
	pop	b
	jmp	map_kernel

vdpoff:
	.word	0

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
	.word	0x03C0		; Start of bottom line
_scrolld_mov:
	.word	0xFFD8		; -40
_scrolld_s1:
	.word	0x4028		; 4000 turns on write 40 chars
_scrollu_w:
	.word	0x0028		; width
_scrollu_mov:
	.word	0x3FD8		; up 40 bytes in low 12, add 0x4000
					; carry 11->12

scrollconf40:
	.word	0x03C0, 0xFFD8, 0x4028, 0x0028, 0x3FD8
scrollconf32:
	.word	0x02E0, 0xFFE0, 0x4020, 0x0020, 0x3FE0
