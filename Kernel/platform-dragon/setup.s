;
;	Raw machine set up for running in 'ROM' mode
;
	.area .startup

	; Optimise all this to use the direct page register as 0xffxx
	orcc #0x10
	; Memory 64K dynamic (I guess...)
	lda 0xffdd
	lda 0xffda
	; Memory low map (cartridge high, RAM low)
	lda 0xffde	; Map type -> cartridge & rom
	; Serial
	lda #0x01
	sta 0xff06
	lda #0x1e
	sta 0xff07

	; 0.9Mhz
	lda 0xffd6	; R0
	lda 0xffd8	; R1
	; Put the video at 0x400
	lda 0xffd2	; F6 to F1 clear F0 set
	lda 0xffd0
	lda 0xffce
	lda 0xffcc
	lda 0xffca
	lda 0xffc8
	lda 0xffc7
	; SAM into ascii mode
	lda 0xffc0
	lda 0xffc2
	lda 0xffc4

	; PIA0 A is all input
	; PIA0 B is all output
	; PIA1 A is mixed, both the same (input 0 only)
	lda #0x4	; enable access to DDR registers
	sta 0xff01
	sta 0xff03
	sta 0xff21
	sta 0xff23
	lda #0x00
	sta 0xff00
	lda #0xff
	sta 0xff02
	lda #0xfe
	sta 0xff21
	sta 0xff23
	lda #0x0	; Disable access to DDR registers
	sta 0xff01
	sta 0xff03
	sta 0xff21
	sta 0xff23

	; VDG via PIA1
	lda #0x00
	ld 0xff22, a	; Graphics, internal text rom

	;
	; Say hello
	;
	ldx 0x400
	lda #'H'
	sta ,x+

