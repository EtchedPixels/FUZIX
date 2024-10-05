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
		; We need to run this with the low 32K ROM mapped
		sta $FFDE		; FIXME: selectable - FFBF for 32K cart
		ldb $80FD		; save from ROM space
		sta $FFDF
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
		jsr _fuzix_main
		orcc #0x10
stop:		bra stop

