		; exported
		.globl start

        	; imported symbols
        	.globl _fuzix_main
	        .globl init_early
	        .globl init_hardware
	        .globl kstack_top
		.globl _system_id

	        ; startup code
	        .area .start

start:
		orcc #0x10		; interrupts definitely off
		lds #kstack_top		; note we'll wipe the stack later
		ldb $80FD		; save from ROM space
		IFDEF MOOH
		lda #64			; enable MMU on MOOH
		sta 0xFF90
		ENDC
		; text and discard may be in memory bank 0
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
		; This might be in BSS so don't save until we've wiped!
		lda #0
		cmpb #0x49		; Dragon32
		beq identified
		inca
		cmpb #0x31		; COCO1/2
		beq identified
		inca
		cmpb #0x32		; COCO3
		beq identified
		inca			; Beats me
identified:
		sta _system_id		; what sort of a box are we ?
		jsr init_early
		jsr init_hardware
		jmp main

		.area .text

main		jsr _fuzix_main
		orcc #0x10		; we should never get here
stop:		bra stop
