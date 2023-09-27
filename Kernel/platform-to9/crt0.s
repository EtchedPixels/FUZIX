		; exported
		.globl start

        	; imported symbols
        	.globl _fuzix_main
	        .globl init_early
	        .globl init_hardware
	        .globl kstack_top

;
;	ROM header
;
	.area	.start

	.ascii " Fuzix For Thomson TO  "
	.byte	0x04		; end of name
	.word	0
	.byte	0xE1		; checksum
	.byte	0x00
	.word	coldboot
	.word	warmboot

coldboot:
warmboot:
	orcc	#0x50
	;	Cartridge bank 0 is 0000-3FFF
	;	Map the rest as we want it
	lda	#$F0	; page 2
	sta	$E7C9
	; Set the video map to bank 0
	lda	0xE7C3
	ora	#0x01
	sta	0xE7C3
	ldx	#0x4000
	ldd	#0xAAAA
wipes:
	std	,x++
	cmpx	#0x6000
	bne	wipes

	; Set the video map to the other bank
	lda	0xE7C3
	anda	#0xFE
	sta	0xE7C3

	; Set mode to 40 column single page
	lda	#0x24
	sta	0xE7DC
	clr	0xE7DD
	; We need to set palette 0 and 2 (bg/fg)
	clra
	sta	0xE7DB		; colour offset 0
	sta	0xE7DA		; colour 0 is black
	sta	0xE7DA
	ldd	#0xFF0F		; white
	sta	0xE7DA
	stb	0xE7DA
	sta	0xE7DA
	stb	0xE7DA

	;	Colours now set
	;	Prepare to copy everything
	ldu	#bootcopy
	ldx	#0x9000
	lds	#0x9FFF
bootcp:
	ldd	,u++
	std	,x++
	cmpu	#bootcopy_end
	bne	bootcp
	;
	;	Install the banks
	;	Cartridge 1 to 0x4000-7FFF
	;	Cartridge 2 to page 2 0xA000-DFFF
	;
	jsr	0x9000
	;
	;	And into the kernel proper
	;
	jmp	main

	;	This code must be relocatable
bootcopy:
	sta	0x1001		; Cartridge bank 1
	ldx	#0x0000
	ldu	#0x4000
copy_1:
	ldd	,x++
	std	,u++
	cmpu	#0x6000
	bne	copy_1

	; Skip the monitor variables
	leax	256,x
	leau	256,u
	; Copy the rest of the 4000-7FFF chunk
copy_2:
	ldd	,x++
	std	,u++
	cmpu	#0x8000
	bne	copy_2

	sta	0x1002		; Cartridge bank 2
	ldx	#0x0000
	ldu	#0xA000
copy_3:
	ldd	,x++
	std	,u++
	cmpu	#0xE000
	bne	copy_3
	sta	0x1000		; Cartridge bank 0
	rts
bootcopy_end:


		.area .discard

main:
	lds	#kstack_top
	ldx	#__sectionbase_.bss__
	ldy	#__sectionlen_.bss__
	clra
bss_wipe:
	sta	,x+
	leay	-1,y
	bne	bss_wipe
	jsr	init_early
	jsr	init_hardware
	jsr	_fuzix_main
	orcc	#0x50
stop:	bra	stop
