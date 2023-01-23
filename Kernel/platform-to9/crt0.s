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
;	At this point the boot loader is in 62xx, our code starts at 6400
;	and is in the fixed mapping for start and discard. interrupts are
;	off (TOOD FIR), sp is in 61xx range
;
;	Video is page1 and the video RAM for display is currently the
;	unmapped half. The machine mode will be TO8/9+. We can address TO9
;	later.
;

start:
		orcc #0x10		; interrupts definitely off
		jmp main

		.area .discard

main:
		lda #$04
		sta $E7E5		; high bank is 4
		lda #$62		
		sta $E7E6		; low bank is 2, from RAM, writeable
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

