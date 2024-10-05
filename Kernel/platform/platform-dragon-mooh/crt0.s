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
		; we enter from bootloader with MMU disabled
		orcc #0x10		; interrupts definitely off
		lds #kstack_top		; note we'll wipe the stack later
		ldb $80FD		; save from ROM space
		lda #4			; MMU bank 4 = KBANKV
		sta 0xFFA0		; set up kernel lower bank
		sta 0xFFA8		; also for user task
		; other banks set by bootloader and DECB poking
		lda #64+8		; enable MMU with CRM on MOOH
		sta 0xFF90
		; use bank 0x3F instead of 3 at 0xE000
		; lotsa point-less copying if only .vectors there...
		lda #3
		sta 0xFFA6		; bank 3 at 0xC000
		lda #0x3F
		sta 0xFFA7		; bank 0x3F in place
		ldx #0xC000		; copy from bank 3
		ldy #0xE000
copyhb		ldu ,x++
		stu ,y++
		cmpx #0xDF00		; or __section* ?
		blo copyhb
		lda #2
		sta 0xFFA6		; bank 2 at 0xC000 again
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
