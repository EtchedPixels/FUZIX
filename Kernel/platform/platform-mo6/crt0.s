	; exported
	.globl start

	; imported symbols
	.globl _fuzix_main
        .globl init_early
        .globl init_hardware
        .globl kstack_top


	.area cartheader

	; Fix this once we figure out checksum rules
	.ascii	"GEMINI"
	.byte	0x04
	.ascii	"18-01-84"
	.byte	0x00
	.word	0
	.word	0
	.word	0x58A2
	.word	0
	.ascii	"DCM05"
	.byte	0
	.word	coldboot


        ; startup code
        .area .start

;
;	Pull ourselves out of cartridge ROM
;	40 col setup for now with half bank TODO change to 80 page properly
;
;	We are running from B000

coldboot:
	ldb	#$14		; Cursor off
	swi
	.byte	0x02

	orcc	#$50
	;	Cartridge bank 0 is 0000-3FFF
	;	Map the rest as we want it
	lda	#$03	; page 3 (we use it for video)
	sta	$A7E5
	; Set the video map to bank 0
	lda	$A7C3
	anda	#$FE
	sta	$A7C3

	ldx	#$6000
	ldd	#$AAAA
wipes:
	std	,x++
	cmpx	#$A000
	bne	wipes

	; Set mode to 80 column
	lda	#$2A
	sta	$A7DC

	; Video from page 3, cart on, border 0
	lda	#$C0
	sta	$A7DD

	; We need to set palette 0 and 1 (bg/fg)
	clra
	sta	$A7DB		; colour offset 0
	sta	$A7DA		; colour 0 is black
	sta	$A7DA
	ldd	#$FF0F		; white
	sta	$A7DA
	stb	$A7DA

	lda	#$02
	sta	$A7E5		; Put page 2 back

	;	Colours now set
	;	Prepare to copy everything
	ldu	#bootcopy
	ldx	#$5000
	lds	#$5FFF
bootcp:
	ldd	,u++
	std	,x++
	cmpu	#bootcopy_end
	bne	bootcp
	;
	;	Install the banks
	;
	jsr	$5000
	;
	;	And into the kernel proper
	;
	jmp	main

	;	This code must be relocatable
bootcopy:
	lda	#$02		; Page 2
	sta	$A7E5

	sta	$B001		; Cartridge bank 1 (6000-9FFF)
	ldx	#$B000
	ldu	#$6000
copy_1:
	ldd	,x++
	std	,u++
	cmpu	#$A000
	bne	copy_1

	sta	$B002		; Cartridge bank 2 (0000-1FFF)

	ldx	#$B000
	ldu	#$0000
copy_2:
	ldd	,x++
	std	,u++
	cmpu	#$2000
	bne	copy_2

				; Cartridge bank 2 (2100-4FFF)

	ldx	#$B100
	ldu	#$2100
copy_3:
	ldd	,x++
	std	,u++
	cmpu	#$5000
	bne	copy_3

	sta	$B000		; Cartridge bank 0

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
	orcc	#$10
stop:	bra	stop

