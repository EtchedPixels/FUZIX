		; exported
		.globl start

        	; imported symbols
        	.globl _fuzix_main
	        .globl init_early
	        .globl init_hardware
	        .globl kstack_top

	        ; startup code
	        .area .start

		; On entry 0000-BFFF are in situ. Cxxx holds the common for
		; Fxxx

		fdb 0x6809
start:
		orcc #0x10		; interrupts definitely off
		lds #kstack_top		; note we'll wipe the stack later

		ldx #$C000
		ldy #$F000
copy1:
		ldd ,x++
		std ,y++
		cmpy #0000
		beq move_done
		; Don't copy into the I/O window
		cmpy #$FE00
		bne copy1
		leax $100,x
		leay $100,y
		bra copy1
move_done:	jmp premain

		.area .discard

premain:
		clra
		ldx #__sectionbase_.udata__
udata_wipe:	sta ,x+
		cmpx #__sectionbase_.udata__+__sectionlen_.udata__
		blo udata_wipe
		ldx #__sectionbase_.bss__
		ldy #__sectionlen_.bss__
bss_wipe:	sta ,x+
		leay -1,y
		bne bss_wipe
		jsr init_early
		jsr init_hardware
		jmp main

		.area .text

main		jsr _fuzix_main
		orcc #0x10		; we should never get here
stop:		bra stop
