        	; imported symbols
        	.globl _fuzix_main
	        .globl init_early
	        .globl init_hardware
	        .globl kstack_top

	        ; startup code
	        .area .start

start:
		jmp main

bootme:
		lda 0xff22		; Switcher is in both images
		anda #0xFB		; at the same address
		sta 0xff22		; ROM switch
		jmp main

		.area .text

main:		orcc #0x10		; interrupts definitely off
		lds #kstack_top

		ldx #__sectionbase_.bss__
		ldy #__sectionlen_.bss__
		clra
bss_wipe:	sta ,x+
		leay -1,y
		bne bss_wipe

		jsr init_early
		jsr init_hardware
		jsr _fuzix_main
		orcc #0x10
stop:		bra stop

