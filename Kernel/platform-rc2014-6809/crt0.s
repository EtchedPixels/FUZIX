		; exported
		.globl start

        	; imported symbols
        	.globl _fuzix_main
	        .globl init_early
	        .globl init_hardware
	        .globl kstack_top

	        ; startup code
	        .area .start

		fdb 0x6809
start:
		orcc #0x10		; interrupts definitely off
		lds #kstack_top		; note we'll wipe the stack later

		ldx #$E000
		ldy #$0
copy1:
		ldd ,--x
		std ,--y
		cmpy #$C000
		beq move_done
		; Don't copy into the I/O window
		cmpy #$FF00
		bne copy1
		leax $-100,x
		leay $-100,y
		bra copy1
move_done:	jmp premain

		.area .discard

		; We pack the data 2K down so we fit under the loader
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
