		; exported
		.globl start

        	; imported symbols
        	.globl _fuzix_main
	        .globl init_early
	        .globl init_hardware
	        .globl kstack_top

	        ; startup code
	        .area .start

start:
		orcc #0x10		; interrupts definitely off
		lds #kstack_top		; note we'll wipe the stack later
		; Undo any remaining ROM map
		jsr map_kernel
		jmp premain

		.area .discard

premain:	clra
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
