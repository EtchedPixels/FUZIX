;
;	We are loaded at 0x0200.
;	0000-7FFF are RAM 8000-FFFF ROM (except I/O)
;
;	We load an image from 0400-FDFF
;	and then jump to 0402 if the marker is right
;
	.setcpu 6303
	.code

	.byte 0x63
	.byte 0x03
start:
	; Map the full 64K to RAM
	ldaa #34
	staa 0xFE7A
	ldaa #35
	staa 0xFE7B
	; Our map is now 32 33 34 35

	lds #0x01FF	; Stack out of the way
	ldx #0x0400

	ldaa #0x01	; 0 is the partition/boot block
	staa sector

	bsr waitready

	ldaa #0xE0
	staa 0xFE16	; Make sure we are in LBA mode
dread:
	bsr waitready
	ldaa #'.'
	bsr outchar
	ldaa sector
	cmpa #0x7D	; loaded all of the image ?
	beq load_done
	inc sector
	staa 0xFE13
	ldaa #0x01
	staa 0xFE12	; num sectors (drives may clear this each I/O)
	bsr waitready
	ldaa #0x20
	staa 0xFE17	; read command

	bsr waitdrq

	clrb
bytes:
	ldaa 0xFE10
	staa ,x
	inx
	ldaa 0xFE10
	staa ,x
	inx
	decb
	bne bytes
	bra dread

load_done:
	ldd 0x0400		; check signature
	cmpa #0x03
	bne bad_load
	cmpb #0x63
	bne bad_load

	ldx #running
	bsr outstring
	jmp 0x0402		; jump to byte following

bad_load:
	ldx #badimg
	bsr outstring
stop:
	bra stop

waitready:
	ldaa 0xFE17
	anda #0x40
	beq waitready
	rts

waitdrq:
	ldaa 0xFE17
	anda #0x09
	beq waitdrq
	rora
	bcc wait_drq_done
	ldaa 0xFE11
	bsr outcharhex
	bra bad_load

wait_drq_done:
	rts

outstring:
	ldaa ,x
	beq outdone1
	bsr outchar
	inx
	jmp outstring

outcharhex:
	tab
	rora
	rora
	rora
	rora
	bsr outcharhex1
	tba
outcharhex1:
	anda #0x0F
	adda #0x30
	cmpa #0x3A
	bcc outchar
	adda #7
outchar:
	psha
outcharw:
	ldaa 0x11	; FIXME: use TIM
	anda #0x20
	beq outcharw
	pula
	staa 0x13
outdone1:
	rts
badimg:
	.byte 13,10
	.ascii "Image not bootable."
running:
	.byte 13,10,0

sector:
	.byte 0
