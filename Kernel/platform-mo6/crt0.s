		; exported
		.globl start

        	; imported symbols
        	.globl _fuzix_main
	        .globl init_early
	        .globl init_hardware
	        .globl kstack_top

	        ; startup code
	        .area .start

;
;	We are booted from a ROM bootstrap. It's then our job
;	to copy ORM bank 1 into memory bank 2, at 0x6000-9FFF
;	and set B000-EFFF to ROM bank 2
;
start:
		orcc #0x10		; interrupts definitely off
		lda #$02
		sta $A7E5		; bank 2
		sta $B001		; switch ROM bank
		ldx #$B000
		ldu #$2000
rom1:
		ldd ,x++		; bank 2 is now loaded with the stuff
		std ,u++		; we bank behind the user page
		cmpu #$E000
		bne  rom1

		sta $B002		; page the next ROM bank in and
					; leave it active

		jmp main

		.area .discard

main:
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

