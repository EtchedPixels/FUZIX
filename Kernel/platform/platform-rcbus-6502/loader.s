;
;	We are loaded at $0200.
;	0000-7FFF are RAM 8000-FFFF ROM (except I/O)
;
;	We switch to all RAM and then load an image from 0400-FDFF
;	and then jump to 4002 if the marker is right
;

	.zeropage
ptr1:	.res	2
sector:	.res	1

	.segment "CODE"

	.byte $65
	.byte $02
start:
	; Map the full 64K to RAM
	lda #34
	sta $FE7A
	lda #35
	sta $FE7B
	; Our map is now 32 33 34 35

	lda #$00
	sta ptr1
	lda #$04
	sta ptr1+1

	lda #$01	; 0 is the partition/boot block
	sta sector

	lda #'['
	jsr outchar
	jsr waitready
	lda #']'
	jsr outchar
	lda #$E0
	sta $FE16	; Make sure we are in LBA mode
dread:
	jsr waitready
	lda #'.'
	jsr outchar
	lda sector
	cmp #$7D	; loaded all of the image ?
	beq load_done
	inc sector
	sta $FE13
	lda #$01
	sta $FE12	; num sectors (drives may clear this each I/O)
	jsr waitready
	lda #$20
	sta $FE17	; read command

	lda #'D'
	jsr outchar
	jsr waitdrq
	lda #'d'
	jsr outchar

	lda ptr1+1	; skip the I/O page
	ldy #0
bytes1:
	lda $FE10
	sta (ptr1),y
	iny
	bne bytes1
	inc ptr1+1
bytes2:
	lda $FE10
	sta (ptr1),y
	iny
	bne bytes2
	inc ptr1+1
	lda #'X'
	jsr outchar
	jmp dread

load_done:
	lda $4000
	cmp #$02
	bne bad_load
	lda $4001
	cmp #$65
	bne bad_load

	ldx #>running
	lda #<running
	jsr outstring
	jmp $4002

bad_load:
	ldx #>badimg
	lda #<badimg
	jsr outstring
	lda $FE16
	jsr outcharhex
	lda $FE15
	jsr outcharhex
	lda $FE14
	jsr outcharhex
	lda $FE13
	jsr outcharhex
stop:
	jmp stop

waitready:
	lda $FE17
	and #$40
	beq waitready
	rts

waitdrq:
	lda $FE17
	and #$09
	beq waitdrq
	and #$01
	beq wait_drq_done
	lda $FE11
	jsr outcharhex
	jmp bad_load

wait_drq_done:
	rts

outstring:
	sta ptr1
	stx ptr1+1
	ldy #0
outstringl:
	lda (ptr1),y
	cmp #0
	beq outdone1
	jsr outchar
	iny
	jmp outstringl

outcharhex:
	tax
	ror
	ror
	ror
	ror
	jsr outcharhex1
	txa
outcharhex1:
	and #$0F
	clc
	adc #'0'
	cmp #'9'+1
	bcc outchar
	adc #7
outchar:
	pha
outcharw:
	lda $FEC5
	and #$20
	beq outcharw
	pla
	sta $FEC0
outdone1:
	rts
badimg:
	.byte 13,10,"Image not bootable."
running:
	.byte 13,10,0
