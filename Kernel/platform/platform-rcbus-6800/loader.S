;
;	We are loaded at 0x0000
;	0000-7FFF are RAM 8000-FFFF ROM (except I/O)
;
;	We load an image from 0100-FFFF
;	and then jump to 0102 if the marker is right
;
	.code

	; So we can check it fits
	.export loader_end
 
	.byte 0x68
	.byte 0x00
start:
	; Map the full 64K to RAM
	ldaa #34
	staa 0xFE7A
	ldaa #35
	staa 0xFE7B
	; Our map is now 32 33 34 35

	lds #0x00FF	; Stack out of the way
	ldx #0x0100

	ldaa #0x01	; 0 is the partition/boot block
	staa @sector

	bsr waitready

	ldaa #0xE0
	staa 0xFE16	; Make sure we are in LBA mode
dread:
	cpx #$0
	beq load_done
	bsr waitready
	ldaa #'.'
	bsr outchar
	ldaa @sector
	inc @sector
	staa 0xFE13
	ldaa #0x01
	staa 0xFE12	; num sectors (drives may clear this each I/O)
	bsr waitready
	ldaa #0x20
	staa 0xFE17	; read command

	bsr waitdrq

	clrb
bytes:
	cpx #$FE00
	bne notio
	ldx #$FF00
notio:
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
	ldaa 0x0100		; check signature
	bne bad_load
	ldab 0x0101
	cmpb #0x68
	bne bad_load

	ldx #running
	bsr outstring
	jmp 0x0102		; jump to byte following

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
	ldaa 0xFEC5
	bita #0x20
	beq outcharw
	pula
	staa 0xFEC0
outdone1:
	rts
outstring:
	ldaa ,x
	beq outdone1
	bsr outchar
	inx
	jmp outstring

badimg:
	.byte 13,10
	.ascii "Image not bootable."
running:
	.byte 13,10,0

loader_end:

	.zp

sector:
	.byte 0
