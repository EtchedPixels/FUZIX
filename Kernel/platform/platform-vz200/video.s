

;	We group our variables with us so they can bank together.

		.module video
		.area _CODE1

		; Imports from C space
		.globl _vtrows
		.globl _vtbottom

m6847_cget:
	push	bc
	push	de
	; Our char is actually an offset in the form
	;	01110CCC CCCC000
	;	So we can just rotate A round 3 and mask off the
	;	bits for high and low, then add the 0x70 to high
	rlca
	rlca
	rlca
	ld	l,a
	and	#7		; within page
	add	#0x70
	ld	h,a
	ld	a,l
	and	#0xF8		; low char bits
	ld	l,a
	ld	de,#ctmp
	;	Copy the symbol into common space
	ld	a,#0x1F		; char bank 256x192
	out	(32),a
	ldi
	ldi
	ldi
	ldi
	ldi
	ldi
	ldi
	ldi
	pop	de
	pop	bc
	ret

ctmp:
	.ds	8

m6847_cwrite:
	;	Write char ctmp into video HL
	;	TODO; optimize
	ld	de,#ctmp
	ld	bc,#32
	ld	a,(de)
	ld	(hl),a
	inc	de
	add	hl,bc
	ld	a,(de)
	ld	(hl),a
	inc	de
	add	hl,bc
	ld	a,(de)
	ld	(hl),a
	inc	de
	add	hl,bc
	ld	a,(de)
	ld	(hl),a
	inc	de
	add	hl,bc
	ld	a,(de)
	ld	(hl),a
	inc	de
	add	hl,bc
	ld	a,(de)
	ld	(hl),a
	inc	de
	add	hl,bc
	ld	a,(de)
	ld	(hl),a
	inc	de
	add	hl,bc
	ld	a,(de)
	ld	(hl),a
	ret

;	Memory for char lines is
;	0111 000BBYYY 000XXXXX
;
;	Y = E X = D
;
m6847_cpos:		;	turn coords DE into address HL
	ld	l,d	;	32x8 so page aligned by character
	ld	a,e	;	Y co-ord
	and	#7
	or	#0x70
	ld	h,a	;	HL is now address
	ld	a,e
	rrca
	rrca
	rrca
	and	#3
	or	#0x1C	;	256x192 mode
	out	(32),a	;	switch video bank
	ret

;	0111 0000 000Y YYYX XXXX

m6847_tpos:
	ld	a,e
	rrca
	rrca
	rrca
	ld	l,a	;	Save for a moment
	and	#0x01
	add	#0x70	;	Address base
	ld	h,a
	ld	a,l
	and	#0xE0
	or	d
	ld	l,a
	ret

	.globl	_plot_char

_plot_char:
	pop	af
	pop	hl
	pop	de
	pop	bc
	push	bc
	push	de
	push	hl
	push	af
	ld	a,(_vidc)
	or	a
	jr	z, plot_text
	ld	a,c
	call	m6847_cget		; character into ctmp
	call	m6847_cpos		; position into HL
	call	m6847_cwrite
	ret
plot_text:
	call	m6847_tpos
	; Remap the char
	; Upper case reverse video, lower case non reverse
	; For punctuation however the other way around
	ld	a,c
	cp	#96		; Characters not in the video map
	jr	c,low_range
	cp	#'z'+1
	jr	c,lowercase
	;	Symbols above Z - subtract and store
	sub	#32
	ld	(hl),a
	ret
lowercase:
	; a-z - subtact 32 invert and store
	sub	#32
inverted:
	or	#0x40
	ld	(hl),a
	ret
low_range:
	; These symbols have matches but handle A-Z separately so it looks
	; nicer visually (with lower case "normal" - ie inverted)
	cp	#'A'
	jr	c, inverted	;	punctuation, digits etc normal
	cp	#'Z'+1
	jr	nc, inverted
	and	#0x3f
	ld	(hl),a
	ret
	
	

;	We need to think hard about video sync and this stuff

scrollbankd:
	out	(32),a
	ld	hl,#0x76FF
	ld	de,#0x77FF
	ld	bc,#0x0700
	lddr
	ret
scrollbanku:
	out	(32),a
	ld	hl,#0x7100
	ld	de,#0x7000
	ld	bc,#0x0700
	ldir
	ret
bankdown:
	;	Move 256 bytes between the banks
	ld	hl,#0x7700	; lowest line of bank
	ld	de,#0x7000	; top line of bank below
	ld	b,#0
bankdown_l:
	out	(32),a		; bank holding source
	ld	c,(hl)
	inc	a		; bank holding destination
	out	(32),a
	dec	a		; back to source
	ex	de,hl
	ld	(hl),c
	ex	de,hl
	inc	hl
	inc	de
	djnz	bankdown_l
	ret
bankup:
	;	Move 256 bytes between the banks
	ld	hl,#0x7000
	ld	de,#0x7700
	ld	b,#0		; 256
	.globl bankup
bankup_l:
	out	(32),a
	ld	c,(hl)
	dec	a
	out	(32),a
	inc	a
	ex	de,hl
	ld	(hl),c
	ex	de,hl
	inc	hl
	inc	de
	djnz	bankup_l
	ret

	.globl	_scroll_down

_scroll_down:
	ld	a,(_vidc)
	or	a
	jr	z, scrd_text
	ld	a,#0x1E
	call	scrollbankd
	ld	a,#0x1D
	call	bankdown
	ld	a,#0x1D
	call	scrollbankd
	ld	a,#0x1C
	call	bankdown
	ld	a,#0x1C
	call	scrollbankd
	ret
scrd_text:
	ld	hl,#0x71FF
	ld	de,#0x71DF
	ld	bc,#0x1E0
	lddr
	ret

	.globl	_scroll_up

_scroll_up:
	ld	a,(_vidc)
	or	a
	jr	z, scru_text
	ld	a,#0x1C
	call	scrollbanku
	ld	a,#0x1D
	call	bankup
	ld	a,#0x1D
	call	scrollbanku
	ld	a,#0x1E
	call	bankup
	ld	a,#0x1E
	call	scrollbanku
	ret
scru_text:
	ld	hl,#0x7020
	ld	de,#0x7000
	ld	bc,#0x01E0
	ldir
	ret

	.globl	_clear_lines

_clear_lines:
	pop	bc
	pop	hl
	pop	de
	push	de
	push	hl
	push	bc
clear_lines:
	;	Clear D lines from E (may be 0)
	ld	a,d
	or	a
	ret	z
	ld	b,d
	ld	d,#0		; X = 0 Y is line passed
	ld	a,(_vidc)
	or	a
	jr	z, clines_t
	;	We recompute for each char line as it may bank flip, but
	;	within a char line is safe
clear_line:
	call	m6847_cpos	; Preserves BCDE
	push	de
	push	bc
	ld	d,h
	ld	e,l
	inc	de
	ld	(hl),#0
	ld	bc,#255
	ldir
	pop	bc
	pop	de
	inc	e
	djnz	clear_line
	ret
clines_t:
	call	m6847_tpos
	ld	c,b		; num lines
	ld	a,#0x60		; inverted space
cline:
	ld	b,#32
cline_t1:
	ld	(hl),a
	inc	hl
	djnz	cline_t1
	dec	c
	jr	nz, cline
	ret

	.globl	_clear_across

_clear_across:
	pop	af
	pop	hl
	pop	de	;	DE = coords
	pop	bc	;	C = count
	push	bc
	push	de
	push	hl
	push	af
	ld	a,c
	or	a	
	ret	z
	ld	a,(_vidc)
	or	a
	jr	z, ca_t
	ret
	call	m6847_cpos
	ld	e,#8
	ld	a,#0
	;	We know the entire row is within the same page
clear_rows:
	ld	b,c
clear_row:
	ld	(hl),a
	inc	hl
	djnz	clear_row
	ld	l,#0	;	pointer to top left of this char block
	inc	h	;	and down a char line (32 x 8)
	dec	e
	jr	nz, clear_rows
	ret
ca_t:
	call	m6847_tpos
	ld	b,c
	ld	a,#0x60	;	inverted space
ca_tl:
	ld	(hl),a
	inc	hl
	djnz	ca_tl
	ret

	.globl	_cursor_on

_cursor_on:
	pop	bc
	pop	hl
	pop	de
	push	de
	push	hl
	push	bc
	ld	(cursorpos),de
cursorinv:
	ld	a,(_vidc)
	or	a
	jr	z, cursorinv_t	
	call	m6847_cpos
	ld	de,#32
	ld	b,#8
cursormake:
	ld	a,(hl)
	cpl
	ld	(hl),a
	add	hl,de
	djnz	cursormake
	ret
cursorinv_t:
	call	m6847_tpos
	ld	a,(hl)
	xor	#0x40
	ld	(hl),a
	ret

	.globl	_cursor_off
; Cache co-ords not address due to banking
_cursor_off:
	ld	de,(cursorpos)
	jr	cursorinv

	.globl	_vtattr_notify
	.globl	_cursor_disable

_vtattr_notify:
_cursor_disable:
	ret

	.globl _video_switch

; We are called once DISCARD is no longer needed. We can now check for video
; banks and if so set up 256x192 video
_video_switch:
	xor	a		; bank 0
	ld	hl,#0x7200
	out	(0x20),a
	ld	(hl),a		; set marker
	inc	a		; set marker in bank 1
	out	(0x20),a
	ld	(hl),a
	dec	a		; and see if it appeared in bank 0
	out	(0x20),a
	cp	(hl)
	ret	nz		; If our write scribbled over the 1C with 1F
				; we are not banked
	ld	a,#0x1c
	out	(0x20),a
	ld	a,#0x08		; graphics on
	ld	(0x6800),a	; graphics latch
	ld	(_vidc),a
	ld	a,#24
	ld	d,a		; clear the lines we have
	ld	(_vtrows),a
	dec	a
	ld	(_vtbottom),a
	ld	e,#0		; from 0,0
	jp	clear_lines	; clear display

	.globl	_do_beep

_do_beep:
	jp	0x3450		; ROM beep will do nicely

	.globl	_vidc

_vidc:
	.db	0		; Text mode default
cursorpos:
	.dw	0
