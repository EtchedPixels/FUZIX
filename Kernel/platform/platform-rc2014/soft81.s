;
;	The loaded image of the ROM is patched so that#
;	0x00 contains a JP to an exit syscall
;	0x2000 contains the ld bc,7fff; jp 0x03CB start up
;	0x027C = C9 (so we don't run video bit of display routine)
;	0x0038-3A is kept with the existing interrupt jp
;	0x0066-68 is kept with the existing nmi jp
;	0x02BB is patched to load HL from a bit above 0x2000 which the
;	kernel side helper keeps with a fake keyboard scan result
;
;	
;

		.area _COMMONMEM
		.module soft81

		.globl _soft_zx81
		.globl _set_kscan

		.globl map_soft81
		.globl map_soft81_restore

		.globl _vdpport
		.globl _inputtty
		.globl _int_disabled

;
;	Soft ZX81 emulation helper. We run off the timer and ideally want to
;	eat a lot of T states
;
;	Our video output is TMS9918A. It's in 32 x 24 character mode with
;	the attributes already set up and the characters set to the ZX81
;	font in the upper half
;
;	We pass in HL holding the process
;
_soft_zx81:
	ld	bc,(_vdpport)
	ld	a,(_inputtty)
	push	af
	push	bc
	call	map_soft81
	ret	z		;	swapped out - can't map

	; HL is the start of the display file
	ld 	hl, (0x400C)
	inc	hl

	pop	bc
	;	Set video position to start of frame buffer memory for this
	;	console
	xor	a
	out	(c),a		;	low byte of address
	pop	af		;	get the input tty back
	dec	a
	add	a
	add	a		;	1K per console
	or	#0x40		;	write mode
	out	(c),a
	dec	c		;	data port
	;
	;	Display pointer is now the top left. As we do a full redraw
	;	we can just scroll across. All this is safe because in our
	;	soft81 mode we indicate no console support so no printing
	;	occurs.
	ld	e,#24
soft_draw:
	ld	b,#32
line_draw:
	ld	a,(hl)
	bit	6,a		;	end marks etc
	jr	nz, line_end_pad
	outi			;	character to display
	jr	nz, line_draw
line_end:
	inc	hl
	dec	e
	jr	nz, soft_draw
	jp	map_soft81_restore
line_end_pad:
	;	Compressed display
	xor	a
	;	May be no padding
	cp	b
	jr	z, line_end
	out	(c),a
	nop
	nop
	nop	; check timing TODO
	djnz	line_end_pad
	jr	line_end

;
;	Per line
;	15 + 7 + 16 + 12 = 50 per character = 1600 per line visible
;	15 + 12 + 4 + 6 + 12 = 49 per line (54 inc ret for final)
;
;	Total ~ 40,000 cycles
;
;	For a real ZX81 fast mode we actual would want to eat 60000 cycles
;	or so to match the CPU clock rate difference, and for slow mode
;	somewhere around 100,000 cycles ever 1/60th (as the VDP is running
;	at the wrong int rate really).
;
;
;	HL is scan code to fake. DI to protect from map_soft81 racing an
;	interrupt that would use it.
;
_set_kscan:
	di
	call	map_soft81
	jr	z, nokey
	ld	(0x2000),hl
	call	map_soft81_restore
nokey:
	ld	a,(_int_disabled)
	or	a
	ret	z
	ei
	ret
