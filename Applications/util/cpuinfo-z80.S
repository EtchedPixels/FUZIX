	.export _ident_asm
	.export _badlibz80_asm
	.export _badt80_asm

	.code

#define CPU_Z80_Z80		0
#define CPU_Z80_Z180		1
#define CPU_Z80_Z280		2
#define CPU_Z80_EZ80		3
#define CPU_Z80_U880		4
#define CPU_Z80_EMULATOR	5
#define CPU_Z80_CLONE		6
#define CPU_Z80_T80		7

_ident_asm:
	xor	a
	ld	e,e		; Seen as .LIL on eZ80
	ld	hl, 0
	inc	a		; part of long load on eZ80 only
	ld	a, CPU_Z80_EZ80
	jr	z, set_id
	xor	a
	dec	a
	daa
	cp	0xF9
	ld	a,CPU_Z80_Z180
	jr	z, set_id
	ld	a, 0x40
	.byte	0xCB,0x37	; from the Z280 data book
	jp	p, z280_detected
	ld	a,(_port_opt)
	or	a
	jr	z, no_port_idwork
	di
	ld	bc,(_port)
	in	e,(c)
	ld	a,1
	.byte	0xED
	.byte	0x71
	/* out (c),0|255 */
	in	a,(c)
	out	(c),e
	ei
	cp	1
	jr	z, emulator_detected
	inc	a
	ld	(_z80_nmos),a
	ld	bc,(_port)
	di
	in	e,(c)
	and	a
	ld	hl,#0x00FF
	outi
	out	(c),e
	ei
	ld	a, 1
	jr	nc, no_port_idwork
	ld	(_outibust), a
no_port_idwork:
	ld	a,(0xffff)
	bit	7,(hl)
	push	af
	pop	bc
	ld	a,c
	and	#0x28
	jr	nz, emulator_detected
	ld	a,(0xfffe)
	bit	7,(hl)
	push	af
	pop	bc
	bit	5,c
	jr	z, emulator_detected
	bit	3,c
	ld	a, CPU_Z80_U880
	jr	z, set_id
	ld	bc,0x00ff
	push	bc
	pop	af	/* Flags is now 0xFF A is 0. Now play with XF and YF */
	scf		/* Will give us 0 for NEC clones, 28 for Zilog */
	nop		/* (Turbo R will also show 28) */
	push	af
	pop	bc
	ld	a,c
	and	0x28
	cp	0x28
	ld	a, #CPU_Z80_Z80
	jr	z, set_id
	ld	a, CPU_Z80_CLONE
	jr	set_id
emulator_detected:
	ld	a,CPU_Z80_EMULATOR
	jr	set_id
	/* TODO: separate Z280 from R800, test the many Z280 bugs */
z280_detected:
	ld	a,CPU_Z80_Z280
set_id:
	ld	(_cpu_id),a
	ret

_badlibz80_asm:
	or 	a	
	ld	hl,0x8000
	ld	bc,0x8000
	adc	hl,bc
	ld	hl,0x0000
	ret	z
	dec	hl
	ret

/* The older T80 core incorrectly fails to set Z on LD A,R when R = 0
   Need to be careful because there's a crappy Z80 emulator out there
   that gets setting the high bit of R wrong */

_badt80_asm:
loop:
	; Loop until R goes to 0
	;
	ld	a,1
	or	a
	ld	a,r
	jr	z, goodz80
	or	a
	nop		; 7 instructions so that we will cycle the full R
	                ; range
	jr	nz, loop
	ld	l,1
	ret
goodz80:
	ld	l,0
	ret
