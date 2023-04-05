;
;	How the chup works, given how incomprehensible the actual datasheet
;	is on first readings:
;
;	Memory layout
;	16K usable RAM.
;
;	Access is via writing a 0x20 + a register number 0-7 + optionally 8
;	to indicate command execution required
;	R0 holds the command/status
;	R1 genernally gets used for the character or argument
;	R2,R3 as arguments
;	R4,R5 and R6,R7 are pointers into the video memory but are not
;	simple addresses
;
;	Mmeory is partitioned oddly to best fit the 40/80 char display
;
;	Each 1K contains a series of 40 byte "buffers"
;	Depending upon the video mode the display line is fetched from
;	either 2 or 3 buffers at the same offset in a blocks n , n + 1, n +
;	2
;
;	The data pointers are
;	Y , top 3 bits give district low bits the row 0 for top status row
;	    1-24 for data rows
;	X , top 2 bits give bank. In 80 columns the bank bits are bits 7,0
;	    and characters alternste banks (ie load the X without shifting)
;
;	Memory is thn partitioned into 8 districts, of 4 blocks, of 1K.
;	Screens must sit within a single district and span multiple blocks
;	All of the screen will be in the same line format
;
;	Each frame consists of 25 lines numbered 0, and 8-31. 8-31 are a
;	hardware scrollable video area. 0 is a status line which can be at
;	the top or bottom. Removing it just makes that line part of the
;	borde ("margin"). The cursor is hardware provided.
;
;	The displayed page is set by DOR7,ROR7-5, and 0 (always even). The
;	lower bits of ROR provide the y row (8-31) at the top so provide 
;	hardware scroll.
;
;	40 column video has two formats
;	24bit (long code)	C is char 0-127 B is the type/set
;				and attributes. A is inverse. flash and RGB
;				fg/bg colours
;				Character sets include ROM sets, UDG sets
;				half res multicolour chars, sixels and 4x2 blocks
;
;	16bit (short code)	reduced features. some latched and chaged on
;				space
;
;	80 column video is rather simpler and has 12bit long codes or 8bit
;	short codes. Short codes are the same as long codes but without the
;	other nybble implicitly 0
;
;	80 char 0-127 are the ROM sets, 128- are 4x2 blocks. No
;	UDGs. The other nybble is negative, flash , underline. colour set
;
;	UDGs are 10x8 and the lowest bit is on the left. They are packed 4
;	per buffer and come in sets of 100 numbered 32-127 and 0-3.
;
;	DOR sets the base for the UDG area. It's a fixed layout
;	1K for alpha , 2K for two banks (odd/even) of 100 semigraphic user
;	8K for all the quadrichrome (half res 4 colour) chars (8 banks)
;
;	Memory map
;	bank 0-3	: console 1
;	bank 4-7	: console 2
;	bank 8-11	: console 3
;	bank 12-15	: console 4
;	UDG banks are not needed for 80 columns
;
	.module ef9345

	; video driver
	.globl _ef_scroll_up
	.globl _ef_scroll_down
	.globl _ef_plot_char
	.globl _ef_clear_lines
	.globl _ef_clear_across
	.globl _ef_cursor_on
	.globl _ef_cursor_off
	.globl _ef_cursor_disable
	.globl _ef_set_console
	.globl _ef9345_init
	.globl _ef9345_probe
	.globl _ef9345_colour
	.globl _ef9345_set_output
	.globl _inputtty
	.globl _outputtty
;
;	Set up
;
;	80 column with attributes
;		TGS	 110C00IN		C = 1 for comp sync
;						I = 1 interlace
;						N = 1 NTSC
;
;	ROR	block origin << 5 | Y origin	bits in weird order for origin
;	PAT	0 ? (check bulk enables)
;	MAT 	0C00?BGR			set border and cursor on off
;		
;	80 char setup
;	char + attributes. Char is 0-127 G0 set, invert flash underline
;							colourset
;	or 128-255 is an bit5 x 2 mosaic code including the attribute bits
;	except D
;	No UDG in 80 char mode
;
	.area _CODE1

waitrdy:
	ld	a,#0x20
	out	(0x44),a
	in	a,(0x46)
	rla
	jr	c, waitrdy
	ret

outreg:
	ld	a,d
	out	(0x44),a
	ld	a,e
	out	(0x46),a
	ret
inreg:
	out	(0x44),a
	in	a,(0x46)
	ret

load_indirect:
	call	waitrdy
	ld	a,#0x21		; Load R1
	out	(0x44), a
	ld	a,e		; Value
	out	(0x46), a
	ld	a,#0x28
	out	(0x44),a	; Load command and execute
	ld	a,d
	or	#0x80		; Write, indirect
	out	(0x46),a
	call	waitrdy
	ret

load_mode:
	ld	d,#1
	ld	b,#5
modereg:
	ld	e,(hl)		; value
	call	load_indirect
	inc	hl
	inc	d
	ld	a,#5
	cp	d
	jr nz, noskip
	inc	d
	inc	d
noskip:
	djnz modereg
	ret

mtab40:
	.byte	0x10		; PAL, combined sync
	.byte	0x4C		; Fixed cursor and enabled I high during margin 
				; margin is blue, no zoom
	.byte	0x7E		; service row off, upper/lower bulk on, conceal on
				; i high during displayed area
				; flash on, 40 col long code
	.byte	0x13		; alpha uds slices in block 3, semi in blocks 2/3
				; quadrichrome in block 0
	.byte	0x28		; origin row 8, display block 0 ; 28 08 ?

mtab80:
	.byte	0xD0		; 80 char PAL combined sync
	.byte	0x48		; Fixed cursor and enabled I high during
				; margin. Margin is blue, no zoom
	.byte	0x7E		; Turn it all on except status bar
	.byte	0x8F		; white on black
	.byte	0x08		; origin row 8 block 0 for display



;
;	Simple probe - try and write/read back AA and 55 patterns then check
;	it fails withut reg 2x
;

ef9345_probef:
	out	(c),b
	ld	a,h
	out	(0x46),a
	out	(c),b
	in	a,(0x46)
	cp	h
	ret

_ef9345_probe:
	ld	hl,#0xAA00
	;
	;	This test should fail as register 0x0X will leave the device
	;	tristated on the bus
	;
	ld	bc,#0x0044	; Tristate
	call	ef9345_probef	; Can we write
	ret	z		; l is 0

	ld	b,#0x21		; Select R1
	;	This text should succeed
	ld	h,#0xAA
	call	ef9345_probef
	ret	nz
	ld	h,#0x55
	call	ef9345_probef
	ret	nz
	inc	l
	ret

_ef9345_init:
	ld	de,#0x2091
	call	outreg		; force a nop
	call	waitrdy		; wait for it to go ready
	ld	hl,#mtab80
	call	load_mode
	ld	hl,#scroll
	ld	(scrollptr),hl
	ld	a,#0x20
	call	wiper
	ld	a,#0x80
	call	wiper
	ld	a,#0xA0
	call	wiper
	xor	a		; current screen
wiper:
	ld	(con_banky),a
	ld	hl,#0		; cursor top left
	call	krl80
	ld	bc,#1920
	xor	a
wipeone:
	call	waitrdy
	ld	de,#0x2920
	call	outreg
	dec	bc
	ld	a,b
	or	c
	jr	nz, wipeone

	; Fall into krl80
;
;	Prepare a KRL command with incrementing
;
krl80:
	ld	hl,(curpos)
	call	waitrdy
	ld	de,#0x2051	; KRL with increment, but do not execute
	call	outreg
	ld	de,#0x2300	; Set attributes (fixed for now)
	call	outreg

	;	Fall into loadxy to set up the cursor

loadxy:
	call	waitrdy
	ld	d,#0x27		; X
	ld	e,h
	rrc	e		; 80 column layout
	call	outreg
	dec	d		; point to Y reg
	; Our real Y position ranges from 8-31 and because we hardware
	; scroll we need to adjust our Y accordingly. 
	;
	; 
	; This needs to be the scroll of the screen we are outputting
	; to
	push	hl
	ld	hl,(scrollptr)
	ld	a,(hl)
	pop	hl
	add	l
	add	#0x08
	cp	#0x20
	jr	c, yok
	sub	#0x18		; Wrapped on hardware scroll
yok:
	; TODO: add in the Z3/Z2 bits from the console code
	ld	h,a
	ld	a,(con_banky)	; console bank info
	add	h
	ld	e,a
	call	outreg
	ret

_ef_plot_char:
	pop	af
	pop	hl
	pop	de	
	pop	bc
	push	bc
	push	de
	push	hl		; D = X E = Y, C = char
	push	af
	ex	de,hl		; H = X L = Y
	; Should check if its next on same line optimise
	call	loadxy
	ld	e,c
	ld	d,#0x29
	call	outreg
	ret

_ef_scroll_down:
	ld	hl, (scrollptr)
	ld	a,(hl)
	dec	a
	cp	#255
	jr	nz, updscroll
	ld	a,#23
updscroll:
	; Needs to be scroll of this tty
	ld	(hl),a
	;	Now check if we are visible
	ld	a,(_inputtty)
	ld	b,a
	ld	a,(_outputtty)
	cp	b
	ret	nz
	; We are visible so update the register
	ld	a,(ror_bank)
	add	(hl)
	ld	d,#7
	add	#8		;  weird offsetting in chip
	ld	e,a
	call	load_indirect
	call	krl80
	ret

_ef_scroll_up:
	ld	hl, (scrollptr)
	ld	a,(hl)
	inc	a
	cp	#24
	jr	nz, updscroll
	xor	a
	jr	updscroll

_ef_clear_lines:
	pop	bc
	pop	hl
	pop	de
	push	de
	push	hl
	push	bc
	;
	; We are in KRL mode so we just need to position and do lots of
	; spaces
	ld	c,d		; C is the line counter
	ld	h,#0		; X is 0
	ld	l,e		; Y into L from E
	;	Now set up at the left of the first line to do
	xor	a
	cp	c
	ret	z
blank_next:
	call	loadxy
	ld	b,#80
blank_line:
	call	waitrdy
	ld	a,#0x29
	out	(0x44),a
	ld	a,#0x20
	out	(0x46),a
	djnz	blank_line
	inc	l
	dec	c
	jr	nz, blank_next
	ret

_ef_clear_across:
	pop	af
	pop	hl
	pop	de
	pop	bc
	push	bc
	push	de	; DE = coords, C = count
	push	hl
	push	af

	ex	de,hl
	call	loadxy
	ld	b,c
	inc	b
	jr	nextacross
spaces:
	call	waitrdy
	ld	a,#0x29
	out	(0x44),a
	ld	a,#0x20
	out	(0x46),a
nextacross:
	djnz	spaces
	ret

_ef_cursor_on:
	pop	bc
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc

	ex	de,hl
	call	waitrdy
	ld	(curpos),hl
	call	loadxy
	ld	a,(cursor)		; already enabled ?
	or	a
	ret	nz
	ld	a, (efmargin)
	or	#0x48
	ld	e,a
	ld	d,#0x02
	call	load_indirect
	call	krl80
	ld	a,#1
	ld	(cursor),a
	ret

_ef_cursor_off:
	ret

_ef_cursor_disable:
	xor	a
	ld	(cursor),a
	ld	a, (efmargin)
	or	#0x08
	ld	e,a
	ld	d,#0x02
direct_res:
	call	load_indirect
	call	krl80
	ret

permute_con:
	rrca				; tty is now in the top 2 bits of A
	rrca
	and	#0xC0
	; Deal with the bonkers ordering in ROR
	; 0-3 but we have a 0 bit in the middle!
	rla
	rl	d			; Z3 into E
	rla				; Z2 is now top bit
	rl	d			; Z2 into E
	rla				; throw out Z1 (0)
	rr	d			; load Z2 back in
	rra
	srl	a			; set Z1 to 0
	rr	d			; load Z3 back in
	rra
	ret

;
;	Sets the console we are writing on
;
_ef9345_set_output:
	;	Set the screen we write to based on outputtty
	ld	a,(_outputtty)
	dec	a
	ld	e,a
	call	permute_con
	;	Save the permuted output console bits
	;	These get merged back in by loadxy
	and	#0xE0
	ld	(con_banky),a
	;	Set the scroll value pointer for this console
	ld	d,#0
	ld	hl,#scroll
	add	hl,de
	ld	(scrollptr),hl
	ret

;
;	Sets the visible console
;
_ef_set_console:
	ld	a,(_inputtty)
	dec	a			; 1 based to 0 based
	call	permute_con
	ld	(ror_bank),a		; Save the permuted bank bits
	ld	e,a			; Save to use in scrolling ROR
	ld	a,(_inputtty)		; Get the scroll bits
	dec	a
	ld	hl,#scroll
	add	l
	ld	l,a
	ld	a,h
	adc	#0
	ld	h,a
	ld	a,(hl)			; Scroll value
	add	#8			; Offset
	or	e			; Merge with bank
	ld	e,a
	ld	d,#7
	call	load_indirect
	call	krl80
	ret

_ef9345_colour:
	pop	hl
	pop	de
	pop	bc
	push	bc
	push	de
	push	hl
	; C is the colour pair
	ld	e,c
	ld	d,#0x4
	call	load_indirect
	ld	a,c
	and	#0x70
	rrca
	rrca
	rrca
	rrca
	ld	(efmargin),a		; for now (40 col is trickier)
	or	#0x08			; I margin
	ld	e,a
	ld	d,#0x02
	ld	a,(cursor)
	or	a
	jr	z, nocur
	set	6,e			; cursor on]
nocur:
	call	load_indirect
	call	krl80
	ret	

	.area _DATA
cursor:
	.byte	0
con_banky:
	.byte	0
scroll:
	.byte	0, 0, 0, 0		; one per console
scrollptr:
	.word	0
efmargin:
	.byte	0
curpos:
	.word	0
ror_bank:
	.byte	0
