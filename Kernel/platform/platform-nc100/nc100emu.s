;
;	Test booter for the NC100 emulator, just makes testing life easier
;

		.area ASEG(ABS)
		.org 0x100
;
;	Switch to the PCMCIA card as if run by FN-x
;
start:		di
		ld a, #0x43		; screen RAM
		out (0x13),a
		ld sp, #stack
		ld hl, #0xf000
		ld de, #0xf001
		ld (hl), #0xAA
		ld bc, #0xFFF
		ldir
		ld a, #0x80		; PCMCIA first bank
		out (0x13), a		; at C000
;		xor a
;		out (0x70),a
		ld a, #0x86		; pcmcia mem, line driver on, uart on
		out (0x30), a		; 9600 baud
		ld a, #'U'
		call chout
		ld a, #'Z'
		call chout
		ld a, #'I'
		call chout
		jp 0xC210		; run

chout:		push af
choutl:		in a, (0xC1)
		bit 0, a
		jr z, choutl
		pop af
		out (0xC0), a
		ret

		.ds 64
stack:
		
