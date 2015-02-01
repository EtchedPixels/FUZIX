;
;	Raw machine set up for running in 'ROM' mode
;
	ORG 0x8000

; Convenient place to stuff the decb block after the bootstrap

data_stream	EQU	0x8400

start:
	jmp main

bootme:
	lda 0xff22
	anda #0xFB
	sta 0xff22		; after this we are in the other rom

main:
	orcc #0x10
	; Optimise all this to use the direct page register as 0xffxx
	lda #0xFF
	tfr a,dp	; So we can short form all our I/O operations

	SETDP 0xFF

;
;	Start by resetting the SAM registers in case we are a soft reset
;
	lda #0x10
	ldx #0xffc0	; SAM registers 0xffc0-0xffdf
samwipe:
	sta ,x++	;
	deca
	bne samwipe

	sta 0xffcf	;
	sta 0xffd1	; Display at 0x6000
	sta 0xffd3	; In the upper bank (= 0xe000)
	;FIXME - DD or DB needed to get it right
	; Memory 64K dynamic (I guess...)
	sta 0xffdd	; 64K dynamic RAM
	sta 0xffd5	; Use RAM bank 1 at 0x0000 (providing ffde is clear)

;	Configure PIAs
	; PIA0 A is all input
	; PIA0 B is all output
	; PIA1 A is mixed, both the same (input 0 only)

	lda #0x04
	sta 0xff23	; Set 0xFF22 to access data register
	sta 0xff22	; Ensure we don't glitch the ROM select bit

	clr 0xff01	; Control to zero - direction accessible
	clr 0xff03
	clr 0xff21	; Do both PIA devices
	clr 0xff23
	clr 0xff00	; PIA0 A all input
	lda #0xff
	sta 0xff02	; Set PIA0 B as output (keyboard matrix)
	lda #0x34
	sta 0xff01
	sta 0xff03

	lda #0xfe
	sta 0xff20	; Set PIA1 A as output except bit 0
			; cassette is an input strobe and DAC output
	sta 0xff22	; Set PIA1 B as output except bit 0
			; 0 printer busy, 1= sound out, 3-7 VDG
	lda #0x4	; Disable access to DDR registers
	sta 0xff21
	sta 0xff23

	; VDG via PIA1 (0), ROM 1
	sta 0xff22	; Graphics, internal text rom

	;
	; Say hello
	;
	ldx #0x6000
cls:	lda #' '
	sta ,x+
	cmpx #0x6200
	bne cls

	ldu #0x6000	; U is our screen pointer
	lds #0x0400	; Somewhere safe out of the way
	ldx #hello
	bsr strout

	;
	;	Clear memory (we ought to tidily clear bits listed in
	;	a tweaked DECB but what the heck..

	ldx #0x6200
wiper:
	clr ,x+
	cmpx #0x8000
	bne wiper
	;
	;	Now the serious stuff - unpack the ROM into RAM
	;	using DECB format
	;

	lda #0
	tfr a,dp

	SETDP 0

	ldy #data_stream
nextblock:
	lda #'#'
	sta ,u+			; progress
	lda ,y+
	bne postamble		; all done
	ldd ,y++		; length
	ldx ,y++		; target
nextbyte:
	pshs a
	lda ,y+
	sta ,x+
	puls a
	subd #1
	cmpd #0
	bne nextbyte
	bra nextblock
postamble:
	inca
	beq loaded_ok
	ldx #fail
	bsr strout
failure:
	bra failure

loaded_ok:
	ldx #loaded
	bsr strout
	jmp bootme

strout:
	lda #10
	cmpa ,x
	bne normch
	leau 31,u
	tfr u,d
	andb #0xE0
	tfr d,u
	leax 1,x
	bra strout
normch:
	lda ,x+
	beq strodone
	anda #0x3F
	sta ,u+
	bra strout
strodone:
	rts

hello:  .ascii "FUZIX DRAGON64 ROM BOOT"
	.byte 10
	.ascii "LOADING BLOCKS: "
	.byte 0
fail:	.byte 10
	.asciz "CORRUPT DECB BLOCK"
loaded:	.byte 10
	.ascii "RAM BLOCKS LOADED"
	.byte 10
	.byte 0


	align 0xBFF0

;
;	Included if we are replacing the basic ROM and magically mapped from
;	the ROM top (0xBFF0->FFF0)
;
	    .dw 0x3634		; Reserved
	    .dw 0x0100		; Make these COCO/Dragon style
	    .dw 0x0103
	    .dw 0x010f
	    .dw 0x010c
	    .dw 0x0106
	    .dw 0x0109
	    .dw main		; Startup vector
